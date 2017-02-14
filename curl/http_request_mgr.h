#pragma once

#include "http_request.h"
#include "http_request_processor.h"
//#include "http_request_results.h"
#include "http_request_common.hpp"
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
	void output_debug(bool enable = false) { output_debug_ = enable; }

	bool hasFreeReq();
	HttpRequest* newReq();
	bool addReq(HttpRequest*);
	void freeReq(HttpRequest*);

	// 获取回调结果
	//bool getResult(HttpResult*& result);
	// 释放回调结果
	//bool freeResult(HttpResult* result);

	void use_thread(bool use = true) { use_thread_ = use; }
	int run();

	static size_t write_callback(char* ptr, size_t size, size_t nmemb, void* userdata);

private:
	int one_loop();
	// 线程函数
	static void thread_func(void*);
	int thread_run();
	void thread_join();

private:
	HttpRequestMgr();
	HttpRequestMgr(const HttpRequestMgr&);
	HttpRequestMgr& operator=(const HttpRequestMgr&);
	
	static HttpRequestMgr* instance_;
	HttpRequestProcessor processor_;
	ThreadSafeObjPool<HttpRequest> pool_;
	ThreadSafeObjList<HttpRequest> list_;
	std::atomic<bool> running_;
	std::thread* work_thread_;
	bool use_thread_;
	bool output_debug_;
};
