// 編譯
// g++ -std=c++11 -pthread hw2_Q1.cpp -o hw2_Q1

// 執行
// ./hw2_Q1 <線程數量> <每個線程當中有幾個點>
// ./hw2_Q1 4 1000000

// 輸出
// 估算的π值: 3.14159265358979323846
// 總點數: 4000000

// 題目：蒙特卡羅方法估算圓周率 π (使用 Pthreads)
// 蒙特卡羅方法: 用大量隨機資料來模擬問題的解答，並從中估算出結果。
// 假設有一個半徑為1的圓(面積為π)，內接於一個邊長為2的正方形(面積為4)中。
// 隨機產生大量的點落在這個正方形內，統計有多少點落在圓內。
// 估計的π值：π ≈ 4 × (圓內點數量) / (總點數量)； 概念相當於 正方形面積 * (圓形面積/正方形面積)。

#include <iostream>	 // 輸入輸出
#include <pthread.h> // pthread多線程庫
#include <random>	 // 隨機數生成
#include <ctime>	 // 時間函數

// 全局變數
unsigned long long points_in_circle = 0;		   // 圓內點數計數器
unsigned long long total_points = 0;			   // 總點數計數器
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // 互斥鎖，用於線程安全

/**
 * 線程函數：生成隨機點並計算圓內點數
 * pthread_create要求線程函數宣告為 void*，並且接受一個 void* 參數。
 * void* 代表可以轉型成任何型別的指標，這樣可以讓我們傳遞任意型別的資料。
 */
void *generate_points(void *arg)
{
	// 把輸入參數強制轉型成所需格式，獲取每個線程點的數量。(只能傳入一個指標，如果想要傳入多個，只能用物件或結構體包起來。)
	unsigned long long num_points = *((unsigned long long *)arg);

	// 建立每個線程單獨的計數器
	unsigned long long local_count = 0;

	// 設置隨機數生成器
	std::random_device rd;							 // 真隨機數設備
	std::mt19937 gen(rd());							 // 使用Mersenne Twister算法
	std::uniform_real_distribution<> dis(-1.0, 1.0); // 均勻分布在[-1,1]

	// 生成隨機點並檢查是否在圓內
	for (unsigned long long i = 0; i < num_points; ++i)
	{
		double x = dis(gen); // 隨機x座標
		double y = dis(gen); // 隨機y座標

		// 檢查點是否在單位圓內 (x² + y² ≤ 1)
		if (x * x + y * y <= 1.0)
		{
			local_count++; // 如果在圓內，本地計數加1
		}
	}

	// 使用互斥鎖安全地更新全局計數
	pthread_mutex_lock(&mutex);		 // 上鎖
	points_in_circle += local_count; // 更新圓內點數
	total_points += num_points;		 // 更新總點數
	pthread_mutex_unlock(&mutex);	 // 解鎖

	return nullptr;
}

int main(int argc, char *argv[])
{
	// 檢查命令行參數
	if (argc != 3)
	{
		std::cerr << "用法: " << argv[0] << " <線程數量> <每個線程當中有幾個點>\n";
		return 1;
	}

	// 解析命令行參數
	int num_threads = std::atoi(argv[1]);						 // 線程數量
	unsigned long long points_per_thread = std::stoull(argv[2]); // 每個線程點的數量

	pthread_t threads[num_threads]; // 創建線程陣列

	// 創建並啟動所有線程
	for (int i = 0; i < num_threads; ++i)
	{
		// pthread_create的參數：線程ID、線程屬性、線程函數、傳遞給線程函數的參數。(參數只能傳入一個指標，如果想要傳入多個，只能用物件或結構體包起來。)
		pthread_create(&threads[i], nullptr, generate_points, &points_per_thread);
	}

	// 等待所有線程完成
	for (int i = 0; i < num_threads; ++i)
	{
		pthread_join(threads[i], nullptr);
	}

	// 計算並輸出π的估算值
	double pi_estimate = 4.0 * points_in_circle / total_points;
	std::cout << "估算的π值: " << pi_estimate << std::endl;
	std::cout << "總點數: " << total_points << std::endl;
	std::cout << "圓內點數: " << points_in_circle << std::endl;

	// 銷毀互斥鎖
	pthread_mutex_destroy(&mutex);

	return 0;
}