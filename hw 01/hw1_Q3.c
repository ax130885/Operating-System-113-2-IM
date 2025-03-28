// 編譯
// gcc hw1_Q3.c -o hw1_Q3 -lrt
// 執行
// ./hw1_Q3 35
// 輸出
// Collatz 序列 (起始值 = 35):
// 35, 106, 53, 160, 80, 40, 20, 10, 5, 16, 8, 4, 2, 1

// 題目：使用 fork() 函數創建子進程，子進程計算 Collatz 序列。
// 透過共享記憶體將結果傳遞給父進程，父進程從共享記憶體讀取結果並印出來。
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>	  // UNIX 標準函式庫 (fork)
#include <sys/wait.h> // 進程等待函式庫
#include <sys/mman.h> // 共享記憶體相關函式庫
#include <sys/stat.h> // shm_open() 和 shm_unlink() 函式庫
#include <fcntl.h>	  // shm_open 時用到的 O_CREAT 和 O_RDWR 常數
#include <string.h>

#define SHM_SIZE 1024 // 共享記憶體大小 單位：byte

int main(int argc, char *argv[])
{
	// 檢查命令行參數
	if (argc != 2)
	{
		fprintf(stderr, "用法：%s <起始正整數>\n", argv[0]);
		return EXIT_FAILURE;
	}

	int start = atoi(argv[1]);
	if (start <= 0)
	{
		fprintf(stderr, "錯誤：必須輸入正整數。\n");
		return EXIT_FAILURE;
	}

	// 1. 開啟或建立共享記憶體
	const char *name = "/collatz_shm";
	// shm_open 打開一個虛擬的共享記憶體的檔案 並且由系統分配一個檔案描述符 (檔案描述符為一個由系統分配的整數，類似於檔案的索引)
	// 此檔案保存在 /dev/shm/ 目錄下，這是一個虛擬檔案，並不會在磁碟上實際存在。
	// 初次創建時要在ftruncate()才會動態分配mmap，但是其他進程在shm_open()時就會動態分配mmap。
	// shm_open(儲存的檔名, O_CREAT如果不存在就創建, O_RDWR可讀(RD)可寫(WR), 0666 (wrx存取權限))
	int shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
	if (shm_fd == -1)
	{
		perror("shm_open 失敗");
		return EXIT_FAILURE;
	}

	// 【單一進程的記憶體佈局】
	// 高地址 0x7ffffffff000
	// ┌───────────────────────────────────────┐
	// │ [核心空間] (用戶進程不可見)            │ ← 所有進程共享的核心記憶體
	// ├───────────────────────────────────────┤ ← 用戶/核心空間分界線
	// │ 0x7ffffffde000 (隨機化起始點)          │
	// │ [主執行緒堆疊]                         │ ← 固定大小區間（預設 8MB）
	// │   實際使用量動態向下增長，但「範圍」固定 │
	// │   觸及邊界時觸發 SIGSEGV               │
	// ├───────────────────────────────────────┤
	// │ [堆疊保護頁] (Guard Page)             │ ← 4KB 不可存取區域
	// ├───────────────────────────────────────┤
	// │ [ASLR 隨機間隙]                       │ ← 128MB~1GB 隨機空隙
	// ├───────────────────────────────────────┤
	// │ [記憶體映射區] (動態向下增長)          │
	// │    ├─ 共享函式庫 (libc.so)            │
	// │    ├─ 檔案映射 (mmap 文件)            │
	// │    ├─ 匿名映射 (malloc 大區塊)        │
	// │    └─ 共享記憶體 (shm)                │
	// ├───────────────────────────────────────┤ ← 只有 Heap 和 mmap 區的虛擬地址邊界會動態變化，其他區段的邊界都是固定的。
	// │ [堆積] (Heap) (動態向上增長)           │ ← 由 brk/sbrk 管理   (malloc在小檔案會使用 brk/sbrk，大檔案會使用 mmap)
	// ├───────────────────────────────────────┤
	// │ [未初始化資料段] (.bss)                │
	// ├───────────────────────────────────────┤
	// │ [已初始化資料段] (.data)               │
	// ├───────────────────────────────────────┤
	// │ [程式碼段] (.text)                    │
	// └───────────────────────────────────────┤
	// 低地址 0x400000

	// 設定共享記憶體的保留大小
	// ftruncate 只有創建時設定一次就好 只需使用時不用再設定
	// ftruncate(shm_open的檔案描述符, 設定共享記憶體的保留大小(byte))
	if (ftruncate(shm_fd, SHM_SIZE) == -1)
	{
		perror("ftruncate 失敗");
		return EXIT_FAILURE;
	}

	// 將設定好的共享記憶體檔案 映射至此進程的虛擬記憶體空間
	/**
	 * @param addr 0，讓系統自動決定進程虛擬記憶體的映射地址。
	 * @param length SHM_SIZE，映射的大小 (單位：byte)。
	 * @param prot PROT_READ | PROT_WRITE，設定映射區域的存取權限 (可讀可寫)。
	 * @param flags MAP_SHARED，共享映射 (多個進程可以訪問同一區域)。
	 * @param fd shm_fd，共享記憶體檔案的檔案描述符。
	 * @param offset 0，地址的偏移量 (通常設為 0)。
	 *
	 * @return shm_ptr 指向共享記憶體的起始地址 (映射後的指標)。失敗時返回 MAP_FAILED。
	 * shm_ptr 可以用任意的指標類型來接收，因為它是 void* 類型。
	 */
	char *shm_ptr = mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	if (shm_ptr == MAP_FAILED)
	{
		perror("mmap 失敗");
		return EXIT_FAILURE;
	}

	// 2. 創建子進程
	pid_t pid = fork();

	if (pid < 0)
	{
		perror("fork 失敗");
		return EXIT_FAILURE;
	}
	else if (pid == 0)
	{ // 子進程
		// 生成 Collatz 序列並寫入共享記憶體
		int n = start;
		// 一個 %c 占用 1 byte
		// 所以可以直接使用 shm_ptr + offset 計算寫入地址；  SHM_SIZE - offset 計算剩餘空間
		int offset = 0;

		/**
		 * snprintf() 將輸入變數轉換為%s後寫入指定的地址，並且檢查記憶體是否溢出(會自動加上結束符'\0')。
		 * 最後回傳想寫入的字元數 (不含結束符) (非實際寫入的字元數)。
		 * @param addr 寫入的地址起點。因為使用shm_ptr + offset，沒有+1，所以只有最後一次寫入的結束符不會被覆蓋。
		 * @param size 要寫入的字串的最大大小 (避免記憶體溢出)。
		 * @param format 指定輸入變數的格式（例如 %s、%d 等）。
		 * @param ... 要寫入的變數。
		 * @return 想寫入的字元數，非實際寫入的字元數 (不包括結束符 '\0')。
		 * 如果 (返回值 +1 > SHM_SIZE - offset) 則表示寫入的字串超過了記憶體的最大大小。
		 * 其中 +1 是為了考慮結束符 '\0' 的大小。
		 */
		offset += snprintf(shm_ptr + offset, SHM_SIZE - offset, "%d", n);

		while (n != 1)
		{
			if (n % 2 == 0) // 如果n是偶數 就除2
			{
				n = n / 2;
			}
			else // 如果n是奇數 就乘3加1
			{
				n = 3 * n + 1;
			}
			// 檢查剩餘空間是否足夠寫入下一個字串(+'\0')，避免記憶體溢出。
			if (offset + 1 >= SHM_SIZE - offset)
			{
				break;
			}
			offset += snprintf(shm_ptr + offset, SHM_SIZE - offset, ", %d", n);
		}

		exit(EXIT_SUCCESS);
	}
	else
	{ // 父進程
		// 3. 等待子進程完成
		wait(NULL);

		// 4. 輸出共享記憶體內容
		printf("Collatz 序列 (起始值 = %d):\n%s\n", start, shm_ptr);

		// 5. 清理共享記憶體
		if (munmap(shm_ptr, SHM_SIZE) == -1)
		{ // 修正語法錯誤
			perror("munmap 失敗");
		}

		// shm_unlink() 用於刪除共享記憶體
		if (shm_unlink(name) == -1)
		{
			perror("shm_unlink 失敗");
		}
	}

	return EXIT_SUCCESS;
}