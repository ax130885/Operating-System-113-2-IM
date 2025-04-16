// 編譯
// g++ -std=c++11 -pthread hw2_Q2.cpp -o hw2_Q2

// 執行
// ./hw2_Q2 <數列長度>
// ./hw2_Q2 10

// 輸出
// 生成的斐波那契數列 (10 個數):
// 0 1 1 2 3 5 8 13 21 34

// 題目：使用多線程(pthread)的方式來生成斐波那契數列(Fibonacci sequence)。
// 父進程僅等待子進程結束，不參與計算，僅印出子進程返回的結果。
#include <iostream>
#include <pthread.h>
#include <vector>

// 定義結構體，用來傳給線程函數。
// 因為pthread_create只能接受一個void*參數，如果要傳多個參數，要用物件或結構體包住。
struct ThreadData
{
	int length;						 // 要生成的斐波那契數列長度
	std::vector<long long> sequence; // 儲存生成的數列
};

// 生成斐波那契數列的線程函數
void *generate_fibonacci(void *arg)
{
	ThreadData *data = (ThreadData *)arg;

	if (data->length >= 1)
	{
		data->sequence.push_back(0); // fib₀ = 0
	}
	if (data->length >= 2)
	{
		data->sequence.push_back(1); // fib₁ = 1
	}

	for (int i = 2; i < data->length; ++i)
	{
		long long next = data->sequence[i - 1] + data->sequence[i - 2];
		data->sequence.push_back(next);
	}

	return nullptr;
}

int main(int argc, char *argv[])
{
	// 檢查命令行參數
	if (argc != 2)
	{
		std::cerr << "用法: " << argv[0] << " <斐波那契數列長度>\n";
		return 1;
	}

	int length = std::atoi(argv[1]);
	if (length < 0)
	{
		std::cerr << "錯誤: 長度必須是非負整數\n";
		return 1;
	}

	// 準備線程資料
	ThreadData data;
	data.length = length;

	pthread_t thread;

	// 創建線程來生成斐波那契數列
	if (pthread_create(&thread, nullptr, generate_fibonacci, &data) != 0)
	{
		std::cerr << "錯誤: 無法創建線程\n";
		return 1;
	}

	// 等待線程完成
	if (pthread_join(thread, nullptr) != 0)
	{
		std::cerr << "錯誤: 無法等待線程\n";
		return 1;
	}

	// 輸出結果
	std::cout << "生成的斐波那契數列 (" << length << " 個數):\n";
	for (long long num : data.sequence)
	{
		std::cout << num << " ";
	}
	std::cout << std::endl;

	return 0;
}