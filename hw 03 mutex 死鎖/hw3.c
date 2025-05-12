// 編譯
// gcc -pthread hw3_Q1.c -o hw3_Q1

// 執行
// ./hw3_Q1

// 輸出
// 估算的π值: 3.1416
// 總點數: 5000
// 圓內點數: 3927

// 題目：蒙特卡羅方法估算圓周率 π (使用 Pthreads) (與hw02 題目相同 差別是固定線程數量和每個線程的點=5 而非從命令列輸入)
// 蒙特卡羅方法: 用大量隨機資料來模擬問題的解答，並從中估算出結果。
// 假設有一個半徑為1的圓(面積為π)，內接於一個邊長為2的正方形(面積為4)中。
// 隨機產生大量的點落在這個正方形內，統計有多少點落在圓內。
// 估計的π值：π ≈ 4 × (圓內點數量) / (總點數量)； 概念相當於 正方形面積 * (圓形面積/正方形面積)。

#include <stdio.h>	 // 標準輸入輸出
#include <pthread.h> // POSIX 線程庫
#include <stdlib.h>	 // 標準庫函數 (rand, srand)
#include <time.h>	 // 時間函數

// 全域變數
unsigned long long points_in_circle = 0;		   // 圓內點數計數器
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // 互斥鎖，用於線程安全

/**
 * 線程函數：生成隨機點並計算圓內點數
 * 每個線程固定生成1000個點
 */
void *generate_points(void *arg)
{
	const int POINTS_PER_THREAD = 1000; // 每個線程固定生成1000個點
	unsigned long long local_count = 0; // 本地計數器

	// 初始化隨機數生成器 (使用線程ID作為種子)
	unsigned int seed = time(NULL) ^ pthread_self();

	for (int i = 0; i < POINTS_PER_THREAD; ++i)
	{
		// 生成[-1, 1]範圍內的隨機數
		double x = (double)rand_r(&seed) / RAND_MAX * 2.0 - 1.0;
		double y = (double)rand_r(&seed) / RAND_MAX * 2.0 - 1.0;

		// 檢查點是否在單位圓內 (x² + y² ≤ 1)
		if (x * x + y * y <= 1.0)
		{
			local_count++; // 如果在圓內，本地計數加1
		}
	}

	// 使用互斥鎖安全地更新全域計數
	pthread_mutex_lock(&mutex);		 // 上鎖
	points_in_circle += local_count; // 更新圓內點數
	pthread_mutex_unlock(&mutex);	 // 解鎖

	return NULL;
}

int main()
{
	const int NUM_THREADS = 5;		// 固定創建5個線程
	pthread_t threads[NUM_THREADS]; // 線程陣列

	// 初始化隨機數種子 (僅用於主線程)
	srand(time(NULL));

	// 創建並啟動所有線程
	for (int i = 0; i < NUM_THREADS; ++i)
	{
		pthread_create(&threads[i], NULL, generate_points, NULL);
	}

	// 等待所有線程完成
	for (int i = 0; i < NUM_THREADS; ++i)
	{
		pthread_join(threads[i], NULL);
	}

	// 計算並輸出π的估算值
	double pi_estimate = 4.0 * points_in_circle / (NUM_THREADS * 1000.0);
	printf("估算的π值: %.15f\n", pi_estimate);
	printf("總點數: %d\n", NUM_THREADS * 1000);
	printf("圓內點數: %llu\n", points_in_circle);

	// 銷毀互斥鎖
	pthread_mutex_destroy(&mutex);

	return 0;
}