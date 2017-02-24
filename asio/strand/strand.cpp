#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <mutex>
#include <iostream>
#include <thread>
#include <chrono>

using namespace boost::asio;

io_service service;
io_service service2;
std::mutex mtx;

void func(int i) {
	std::lock_guard<std::mutex> g(mtx);
	std::cout << "func called i=" << i << "/" << std::this_thread::get_id() << std::endl;
}

void worker_thread() {
	service.run();
}

int main(int argc, char* argv[]) {
	(void)argc;
	(void)argv;

	io_service::strand strand_one(service), strand_two(service);
	for (int i=0; i<5; i++) {
		service.post(strand_one.wrap(std::bind(func, i)));
	}
	for (int i=5; i<10; i++) {
		service.post(strand_two.wrap(std::bind(func, i)));
	}
	boost::thread_group threads;
	for (int i=0; i<3; i++) {
		threads.create_thread(worker_thread);
	}

	std::this_thread::sleep_for(std::chrono::seconds(1));
	threads.join_all();
}
