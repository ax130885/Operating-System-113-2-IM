// 編譯
// g++ -std=c++11 -pthread hw3_Q3.cpp -o hw3_Q3

// 執行
// ./hw3_Q3

// 輸出
// 原始陣列: 7 12 19 3 18 4 2 6 15 8
// 部分排序後: 3 7 12 18 19 2 4 6 8 15
// 完全排序後: 2 3 4 6 7 8 12 15 18 19

// 題目：輸入一個列表，將列表分為一半，丟到兩個線程中進行排序。
// 然後再用一個線程，合併兩個已排序的列表。
// 父線程僅在最後印出結果。
#include <iostream>
#include <pthread.h>
#include <vector>
#include <algorithm>

// 全局共享資料
std::vector<int> original_array = {7, 12, 19, 3, 18, 4, 2, 6, 15, 8};
std::vector<int> sorted_array(original_array.size());

// 排序起訖結構體
struct SortParams
{
	size_t start_index;
	size_t end_index;
};

// 排序線程函數
void *sort_thread(void *arg)
{
	SortParams *params = (SortParams *)arg;

	// 使用 std::sort (通常是 quick sort) 對指定範圍的子陣列進行排序
	std::sort(original_array.begin() + params->start_index,
			  original_array.begin() + params->end_index + 1);

	return nullptr;
}

// 合併線程函數
void *merge_thread(void *arg)
{
	size_t mid = original_array.size() / 2 - 1;
	size_t i = 0;		// 左半部分索引
	size_t j = mid + 1; // 右半部分索引
	size_t k = 0;		// 合併陣列索引

	// 合併兩個已排序的子陣列
	while (i <= mid && j < original_array.size())
	{
		if (original_array[i] <= original_array[j])
		{
			sorted_array[k++] = original_array[i++];
		}
		else
		{
			sorted_array[k++] = original_array[j++];
		}
	}

	// 複製剩餘元素
	while (i <= mid)
	{
		sorted_array[k++] = original_array[i++];
	}

	while (j < original_array.size())
	{
		sorted_array[k++] = original_array[j++];
	}

	return nullptr;
}

int main()
{
	// 顯示原始陣列
	std::cout << "原始陣列: ";
	for (int num : original_array)
	{
		std::cout << num << " ";
	}
	std::cout << std::endl;

	// 計算分割點
	size_t mid = original_array.size() / 2 - 1; // size_t 是無號整數 用於表示索引

	// 創建排序線程參數
	SortParams params1 = {0, mid};
	SortParams params2 = {mid + 1, original_array.size() - 1};

	pthread_t sort_thread1, sort_thread2, merge_thread1;

	// 創建並啟動排序線程
	pthread_create(&sort_thread1, nullptr, sort_thread, &params1);
	pthread_create(&sort_thread2, nullptr, sort_thread, &params2);

	// 等待排序線程完成
	pthread_join(sort_thread1, nullptr);
	pthread_join(sort_thread2, nullptr);

	// 顯示部分排序結果
	std::cout << "部分排序後: ";
	for (int num : original_array)
	{
		std::cout << num << " ";
	}
	std::cout << std::endl;

	// 創建並啟動合併線程
	pthread_create(&merge_thread1, nullptr, merge_thread, nullptr);

	// 等待合併線程完成
	pthread_join(merge_thread1, nullptr);

	// 輸出最終排序結果
	std::cout << "完全排序後: ";
	for (int num : sorted_array)
	{
		std::cout << num << " ";
	}
	std::cout << std::endl;

	return 0;
}