#pragma once

#include <boost/lockfree/queue.hpp>
#include <boost/pool/pool_alloc.hpp>
#include "thread_safe_obj_pool.hpp"
#include "thread_safe_obj_list.hpp"

class HttpRequestResults
{
public:
	HttpRequestResults();
	~HttpRequestResults();

	void init(int size);
	void clear();
	// 只能在同一个线程里调用
	bool insertResult(const char*, int);
	// 获取一个结果
	bool popResult(char*&, int&);
	// 在另一个线程里使用，并非真正回收，而是放入一个队列
	bool freeResult(char*, int);
	// 循环调用释放回收的内存，与分配内存的是同一个线程
	void doLoop();

private:
	void deallocResults();
	void deallocToFreeResults();

private:
	struct Result {
		const char* str;
		int len;
	};
	boost::lockfree::queue<Result> results_;
	boost::lockfree::queue<Result> to_free_results_;
	boost::pool_allocator<char> results_alloc_;
};
