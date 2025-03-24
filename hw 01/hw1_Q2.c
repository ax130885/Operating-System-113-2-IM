// 編譯
// gcc hw1_Q2.c -o hw1_Q2 -lrt
// 執行
// ./hw1_Q2 35
// 輸出
// 子進程：Collatz 序列 (起始值 = 35):
// 35, 106, 53, 160, 80, 40, 20, 10, 5, 16, 8, 4, 2, 1
// 父進程：子進程已完成。

// 題目：使用 fork() 函數創建子進程，子進程生成並打印 Collatz 序列，父進程等待子進程結束後打印提示信息。
#include <stdio.h>	  // 標準輸入輸出
#include <stdlib.h>	  // 標準函式庫 (atoi, exit)
#include <unistd.h>	  // UNIX 標準函式庫 (fork)
#include <sys/wait.h> // 進程等待函式庫

/**
 * 生成並打印 Collatz 序列
 * @param n 起始正整數
 */
void generate_collatz(int n)
{
	// 檢查輸入是否為正整數
	if (n <= 0)
	{
		fprintf(stderr, "錯誤：必須輸入正整數。\n");
		exit(EXIT_FAILURE);
	}

	printf("子進程：Collatz 序列 (起始值 = %d):\n", n);
	printf("%d", n); // 先印出第一個數字

	while (n != 1)
	{
		if (n % 2 == 0)
		{
			n = n / 2; // 偶數情況
		}
		else
		{
			n = 3 * n + 1; // 奇數情況
		}
		printf(", %d", n); // 印出序列中的每個數字
	}
	printf("\n"); // 最後換行
}

int main(int argc, char *argv[])
{
	// 檢查命令行參數數量
	if (argc != 2)
	{
		fprintf(stderr, "用法：%s <起始正整數>\n", argv[0]);
		return EXIT_FAILURE;
	}

	// 將字串參數轉為整數
	int n = atoi(argv[1]);

	// 創建子進程
	pid_t pid = fork();

	if (pid < 0)
	{
		// fork() 失敗
		fprintf(stderr, "創建子進程失敗\n");
		return EXIT_FAILURE;
	}
	else if (pid == 0)
	{
		// 子進程
		generate_collatz(n);
		exit(EXIT_SUCCESS);
	}
	else
	{
		// 父進程
		wait(NULL); // 等待子進程完成
		printf("父進程：子進程已完成。\n");
	}

	return EXIT_SUCCESS;
}