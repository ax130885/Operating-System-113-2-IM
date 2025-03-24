// 編譯
// gcc hw1_Q3.c -o hw1_Q3 -lrt
// 執行
// ./hw1_Q3 35
// 輸出
// Collatz 序列 (起始值 = 35):
// 35, 106, 53, 160, 80, 40, 20, 10, 5, 16, 8, 4, 2, 1

// 題目：使用 fork() 函數創建子進程，子進程生成並打印 Collatz 序列。透過共享記憶體將結果傳遞給父進程，父進程等待子進程結束後打印序列。
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define SHM_SIZE 1024 // 共享記憶體大小

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

	// 1. 建立共享記憶體物件
	const char *name = "/collatz_shm";
	int shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
	if (shm_fd == -1)
	{
		perror("shm_open 失敗");
		return EXIT_FAILURE;
	}

	// 設定共享記憶體大小
	if (ftruncate(shm_fd, SHM_SIZE) == -1)
	{
		perror("ftruncate 失敗");
		return EXIT_FAILURE;
	}

	// 映射共享記憶體
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
		int offset = 0;

		// 使用 snprintf 安全地寫入共享記憶體
		offset += snprintf(shm_ptr + offset, SHM_SIZE - offset, "%d", n);

		while (n != 1)
		{
			if (n % 2 == 0)
			{
				n = n / 2;
			}
			else
			{
				n = 3 * n + 1;
			}
			offset += snprintf(shm_ptr + offset, SHM_SIZE - offset, ", %d", n);

			// 防止緩衝區溢出
			if (offset >= SHM_SIZE - 20)
			{
				break;
			}
		}

		// 結束標記
		shm_ptr[offset] = '\0';
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

		if (shm_unlink(name) == -1)
		{
			perror("shm_unlink 失敗");
		}
	}

	return EXIT_SUCCESS;
}