// 編譯
// gcc hw1_Q1.c -o hw1_Q1 -lrt
// 執行
// ./hw1_Q1 7
// 輸出
// 子進程：前 7 項斐波那契數列：
// 0 1 1 2 3 5 8
// 父進程：子進程已完成。

// 題目：使用 fork() 函數創建子進程，子進程生成並打印前 n 項斐波那契數列，父進程等待子進程結束後打印提示信息。
#include <stdio.h>	  // 標準輸入輸出函式庫
#include <stdlib.h>	  // 標準函式庫（包含 atoi() 和 exit()）
#include <unistd.h>	  // UNIX 標準函式庫（包含 fork()）
#include <sys/wait.h> // 進程等待相關函式庫

/**
 * 生成斐波那契數列並打印
 * @param n 要生成的斐波那契數列項數
 */
void generate_fibonacci(int n)
{
	// 檢查輸入是否為負數
	if (n < 0)
	{
		fprintf(stderr, "錯誤：項數必須是非負數。\n");
		exit(EXIT_FAILURE); // 異常退出
	}

	printf("子進程：前 %d 項斐波那契數列：\n", n);

	// 處理前兩項特殊情況
	if (n >= 1)
		printf("%d ", 0); // 第1項：0
	if (n >= 2)
		printf("%d ", 1); // 第2項：1

	// 初始化變數，使用 long 型別以防大數
	long a = 0, b = 1;
	// 從第3項開始計算
	for (int i = 3; i <= n; i++)
	{
		long next = a + b;	  // 計算下一項
		printf("%ld ", next); // 打印當前項
		a = b;				  // 更新前前一項
		b = next;			  // 更新前一項
	}
	printf("\n"); // 換行
}

int main(int argc, char *argv[])
{
	// 檢查命令行參數數量
	if (argc != 2)
	{
		fprintf(stderr, "用法：%s <項數>\n", argv[0]);
		return EXIT_FAILURE;
	}

	// 將字符串參數轉換為整數
	int n = atoi(argv[1]);
	// 再次檢查是否為負數
	if (n < 0)
	{
		fprintf(stderr, "錯誤：項數必須是非負數。\n");
		return EXIT_FAILURE;
	}

	// 創建子進程
	pid_t pid = fork();

	if (pid < 0)
	{
		// fork() 失敗的情況
		fprintf(stderr, "創建子進程失敗\n");
		return EXIT_FAILURE;
	}
	else if (pid == 0)
	{
		// 子進程執行的代碼
		generate_fibonacci(n); // 生成斐波那契數列
		exit(EXIT_SUCCESS);	   // 子進程正常退出
	}
	else
	{
		// 父進程執行的代碼
		wait(NULL); // 等待子進程結束
		printf("父進程：子進程已完成。\n");
	}

	return EXIT_SUCCESS; // 程序正常結束
}