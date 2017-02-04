#pragma once

#include "http_request.h"
#include "http_request_processor.h"
#include "http_request_pool.h"
#include <atomic>

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
	// 主线程调用
	int run();
	// 线程函数
	static void thread_func(void*);
	// 回调处理
	static size_t write_callback(char* ptr, size_t size, size_t nmemb, void* userdata);

private:
	HttpRequestMgr();
	HttpRequestMgr(const HttpRequestMgr&);
	HttpRequestMgr& operator=(const HttpRequestMgr&);
	
	static HttpRequestMgr* instance_;
	HttpRequestProcessor processor_;
	HttpRequestPool pool_;
	std::atomic<bool> running_;
};
