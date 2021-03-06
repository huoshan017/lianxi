#pragma once

#include "http_request.h"
#include "http_request_processor.h"
#include "http_request_common.hpp"
#include <atomic>
#include <boost/lockfree/queue.hpp>
#include <boost/pool/pool_alloc.hpp>
#include <thread>

class HttpRequestMgr
{
public:
	HttpRequestMgr();
	~HttpRequestMgr();

	bool init(int max_request = 0);
	void close();
	void setOutputDebug(bool enable = false);
	void setStatistics(bool enable = false);

	bool hasReq();
	HttpRequest* newReq();
	bool addReq(HttpRequest*);
	void freeReq(HttpRequest*);

#if USE_RESPONSE_UNITY
	int get(const char* url, http_resp_func cb_func = nullptr, void* func_param = nullptr);
	int post(const char* url, const char* post_content, http_resp_func cb_func = nullptr, void* func_param = nullptr);
#else
	// GET request
	int get(const char* url, http_resp_func cb_func = nullptr, void* func_param = nullptr, http_error_func err_func = nullptr, void* err_param = nullptr);
	// POST request
	int post(const char* url, const char* post_content, http_resp_func cb_func = nullptr, void* func_param = nullptr, http_error_func err_func = nullptr, void* err_param = nullptr);
#endif
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
	HttpRequestProcessor processor_;
	ThreadSafeObjPool<HttpRequest> pool_;
	ThreadSafeObjList<HttpRequest> list_;
	std::atomic<bool> running_;
	std::thread* work_thread_;
	bool use_thread_;
	bool output_debug_;
	bool enable_statistics_;
};
