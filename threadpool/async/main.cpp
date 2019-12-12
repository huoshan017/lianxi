#include <thread>
#include <iostream>
#include <future>
#include <chrono>

struct MyData
{
	int value;
	float conf;
};

MyData data{ 0, 0.0f };

int main()
{
	auto start = std::chrono::steady_clock::now();
	std::future<MyData> dataFuture = std::async(std::launch::async, []() -> MyData {
		std::this_thread::sleep_for(std::chrono::seconds(2));
		return MyData{ 2, 1 };
	});

	std::this_thread::sleep_for(std::chrono::seconds(1));
	auto res = dataFuture.get();
	std::cout << res.value << "\t" << res.conf << std::endl;

	auto end = std::chrono::steady_clock::now();
	std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << std::endl;

	return 0;
}