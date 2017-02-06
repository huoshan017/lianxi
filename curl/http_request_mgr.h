#pragma once

#include "http_request.h"
#include "http_request_processor.h"
#include "http_request_results.h"
#include "thread_safe_obj_pool.hpp"
#include "thread_safe_obj_list.hpp"
#include <atomic>
#include <boost/lockfree/queue.hpp>
#include <boost/pool/pool_alloc.hpp>

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
	bool getResult(char*&, int&);
	// 释放回调结果
	bool freeResult(char*, int);

	// 主线程调用
	int run();

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
	//HttpRequestPool pool_;
	ThreadSafeObjPool<HttpRequest> pool_;
	ThreadSafeObjList<HttpRequest> list_;
	std::atomic<bool> running_;
	HttpRequestResults results_;
};
