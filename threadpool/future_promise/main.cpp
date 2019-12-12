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
	std::promise<MyData> dataPromise;
	std::future<MyData> dataFuture = dataPromise.get_future();

	std::thread producer(
		[&](std::promise<MyData> &data) -> void {
			std::this_thread::sleep_for(std::chrono::seconds(1));
			data.set_value({ 2, 1.0f });
		},
		std::ref(dataPromise)
	);

	std::thread consumer(
		[&](std::future<MyData> &data) -> void {
			auto a = data.valid();
			std::cout << a << std::endl;
			auto res = data.get();
			std::cout << res.value << "\t" << res.conf << std::endl;
			auto b = data.valid();
			std::cout << b << std::endl;
		},
		std::ref(dataFuture)
	);

	producer.join();
	consumer.join();

	return 0;
}