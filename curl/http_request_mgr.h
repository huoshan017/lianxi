#pragma once

#include "http_request.h"
#include "http_request_processor.h"
#include "http_request_results.h"
#include "http_request_common.h"
#include <atomic>
#include <boost/lockfree/queue.hpp>
#include <boost/pool/pool_alloc.hpp>
#include <thread>

class HttpRequestMgr
{
public:
	~HttpRequestMgr();
	static HttpRequestMgr* getInstance();

	bool init(int max_request = 0);
	void close();

	bool hasFreeReq();
	HttpRequest* newReq();
	bool addReq(HttpRequest*);
	void freeReq(HttpRequest*);

	// 获取回调结果
	bool getResult(HttpResult*& result);
	// 释放回调结果
	bool freeResult(HttpResult* result);

	int one_loop();
	void run();
	int thread_run();
	void thread_join();

	// 线程函数
	static void thread_func(void*);
	// 回调处理在线程中
	static size_t write_callback(char* ptr, size_t size, size_t nmemb, void* userdata);

private:
	HttpRequestMgr();
	HttpRequestMgr(const HttpRequestMgr&);
	HttpRequestMgr& operator=(const HttpRequestMgr&);
	
	static HttpRequestMgr* instance_;
	HttpRequestProcessor processor_;
	ThreadSafeObjPool<HttpRequest> pool_;
	ThreadSafeObjList<HttpRequest> list_;
	std::atomic<bool> running_;
	HttpRequestResults results_;
	std::thread* work_thread_;
};
