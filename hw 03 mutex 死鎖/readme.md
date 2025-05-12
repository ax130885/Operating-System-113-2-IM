<!-- title: NTU IM Operating-System HW 02 -->
---
Title: NTU IM Operating-System HW 023 
Student ID: R12631070  
Name: 林育新  
---

# 問答題 (6.14)

# 程式題 hw3.c (6.33)
題目: 蒙特卡羅方法估算圓周率 π (使用 Pthreads)  
1. **使用方式**  
	```bash
	Make
	./hw3
	```

2. **算法解釋**：  
	蒙特卡羅方法: 用大量隨機資料來模擬問題的解答，並從中估算出結果。  
	假設有一個半徑為1的圓(面積為π)，內接於一個邊長為2的正方形(面積為4)中。  
	隨機產生大量的點落在這個正方形內，統計有多少點落在圓內。  
	估計的π值：π ≈ 4 × (圓內點數量) / (總點數量)； 概念相當於 正方形面積 * (圓形面積/正方形面積)。  

3. **子線程邏輯**：  
   - 生成隨機點並計算圓內點數，結束前用mutex更新global的point數量。
		```cpp
		void *generate_points(void *arg)
		{
			// 把輸入參數強制轉型成所需格式，獲取每個線程點的數量。(只能傳入一個指標，如果想要傳入多個，只能用物件或結構體包起來。)
			unsigned long long num_points = *((unsigned long long *)arg);
			
			// 建立每個線程單獨的計數器
			unsigned long long local_count = 0;

			// 設置隨機數生成器
			std::random_device rd;                             // 真隨機數設備
			std::mt19937 gen(rd());                            // 使用Mersenne Twister算法
			std::uniform_real_distribution<> dis(-1.0, 1.0);  // 均勻分布在[-1,1]

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
			pthread_mutex_lock(&mutex);         // 上鎖
			points_in_circle += local_count;    // 更新圓內點數
			total_points += num_points;         // 更新總點數
			pthread_mutex_unlock(&mutex);       // 解鎖

			return nullptr;
		}
		```
4. **主線程邏輯**：
   - pthread_create 創建線程。
   - pthread_join 等待所有子線程完成以後，計算π的估計值。
		```cpp
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
		```




