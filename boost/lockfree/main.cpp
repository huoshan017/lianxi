#include <boost/lockfree/queue.hpp>
#include <boost/atomic/atomic.hpp>
#include <boost/thread/thread.hpp>
#include <iostream>

boost::atomic_int producer_count(0);
boost::atomic_int consumer_count(0);
boost::lockfree::queue<int> queue(512);
// 是否生产完毕标志
boost::atomic<bool> done(false);

// 迭代次数
const int iterations = 1000000;
// 生产线程数
const int producer_thread_count = 4;
// 消费线程数
const int consumer_thread_count = 2;

// 生产函数
void producer()
{
	int i = 0;
	for (; i!=iterations; i++) {
		// 原子计数---多线程不存在计数不上的情况
		int value = ++producer_count;
		std::cout << "*"; // 观察生产类型， 纯生产还是同时有消费的情况
		// 若没有进入队列，则重复推送
		while (!queue.push(value));
	}
}

// 消费函数
void consumer()
{
	int value;
	// 当没有生产完毕，则边消费边生产
	while (!done) {
		while (queue.pop(value)) {
			std::cout << "."; // 观察消费类型，纯消费还是边生产边消费
			++consumer_count;
		}
	}
	// 如果生产完毕则消费
	while (queue.pop(value)) {
		++consumer_count;
	}
}

int main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;

	std::cout << "boost::lockfree::queue is ";
	if (!queue.is_lock_free())
		std::cout << "not";
	std::cout << "lockfree" << std::endl;

	// 线程群管理器
	boost::thread_group producer_threads, consumer_threads;
	// 创建生产者线程
	for (int i=0; i!=producer_thread_count; i++) {
		producer_threads.create_thread(producer);
	}
	// 创建消费者线程
	for (int i=0; i!=consumer_thread_count; i++) {
		consumer_threads.create_thread(consumer);
	}

	// 等待生产者生产完毕
	producer_threads.join_all();
	// 可以消费标志
	done = true;

	std::cout << "done" << std::endl;
	// 等待消费者结束
	consumer_threads.join_all();

	// 输出生产者消费者数量
	std::cout << "produced" << producer_count << " objects." << std::endl;
	std::cout << "consumed" << consumer_count << " objects." << std::endl;  

	return 0;
}
