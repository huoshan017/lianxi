#pragma once

#include <boost/lockfree/queue.hpp>
#include <boost/pool/pool_alloc.hpp>
#include "http_request_common.hpp"

class HttpRequest;

struct HttpResult {
	const char* str;
	int len;
	HttpRequest* req;
	HttpResult() : str(NULL), len(0), req(NULL) {}
};

class HttpRequestResults
{
public:
	HttpRequestResults();
	~HttpRequestResults();

	void init(int size);
	void clear();
	// 只能在同一个线程里调用
	bool insertResult(char*, int, HttpRequest*);
	// 获取一个结果
	bool popResult(HttpResult*&);
	// 在另一个线程里使用，并非真正回收，而是放入一个队列
	bool freeResult(HttpResult*);
	// 循环调用释放回收的内存，与分配内存的是同一个线程
	void doLoop();

private:
	void deallocResults();
	void deallocToFreeResults();

private:
	
	//ThreadSafeObjPool<HttpResult> results_pool_;
	ThreadSafeObjList<HttpResult> results_;
	ThreadSafeObjList<HttpResult> to_free_results_;
	boost::pool_allocator<char> results_alloc_;
};
