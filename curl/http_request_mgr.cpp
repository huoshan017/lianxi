#include "http_request_mgr.h"
#include <thread>
#include <chrono>
#ifndef WIN32
#include <unistd.h>
#else
#include <windows.h>
#endif
#include <iostream>
#include <memory.h>

const static int DEFAULT_MAX_REQUEST_COUNT = 5000;

HttpRequestMgr::HttpRequestMgr() : running_(false), work_thread_(NULL), use_thread_(false), output_debug_(false), enable_statistics_(false)
{
}

HttpRequestMgr::~HttpRequestMgr()
{
	close();
}

bool HttpRequestMgr::init(int max_nreq)
{
	if (max_nreq <= 0) {
		max_nreq = DEFAULT_MAX_REQUEST_COUNT;
	}

	if (!processor_.init(DEFAULT_MAX_PROCESS_COUNT))
		return false;

	processor_.setMgr(this);

	list_.init(max_nreq);

	if (!pool_.init(max_nreq, false))
		return false;

	return true;
}

void HttpRequestMgr::close()
{
	pool_.clear();
	processor_.close();
	running_ = false;
	if (use_thread_) {
		thread_join();
	}
	use_thread_ = false;
}

void HttpRequestMgr::setOutputDebug(bool enable)
{
	output_debug_ = enable;
}

void HttpRequestMgr::setStatistics(bool enable)
{
	enable_statistics_ = enable;
	processor_.setStatistics(enable);
}

bool HttpRequestMgr::hasReq()
{
	return pool_.notEmpty();
}

HttpRequest* HttpRequestMgr::newReq()
{
	HttpRequest* req = pool_.malloc();
	if (!req)
		return NULL;

	if (!req->init())
		return NULL;
	
	return req;
}

bool HttpRequestMgr::addReq(HttpRequest* req)
{
	if (!req) return false;
	return list_.push(req);
}

void HttpRequestMgr::freeReq(HttpRequest* req)
{
	req->close();
	pool_.free(req);
}

// GET request
#if USE_RESPONSE_UNITY
int HttpRequestMgr::get(const char* url, http_resp_func cb_func, void* func_param)
#else
int HttpRequestMgr::get(const char* url, http_resp_func cb_func, void* func_param, http_error_func err_func, void* err_param)
#endif
{
	HttpRequest* req = newReq();
	if (!req) {
		if (output_debug_)
			std::cout << "newReq failed" << std::endl;
		return 0;
	}

	req->setUrl(url);
	req->setGet(true);

	req->setRespWriteFunc(cb_func, func_param);
#if !USE_RESPONSE_UNITY
	req->setErrorFunc(err_func, err_param);
#endif

	if (!addReq(req)) {
		freeReq(req);
		if (output_debug_)
			std::cout << "add req failed" << std::endl;
		return -1;
	}

	return 1;
}

// POST request
#if USE_RESPONSE_UNITY
int HttpRequestMgr::post(const char* url, const char* post_content, http_resp_func cb_func, void* func_param)
#else
int HttpRequestMgr::post(const char* url, const char* post_content, http_resp_func cb_func, void* func_param, http_error_func err_func, void* err_param)
#endif
{
	HttpRequest* req = newReq();
	if (!req) {
		if (output_debug_)
			std::cout << "newReq failed" << std::endl;
		return 0;
	}

	req->setUrl(url);
	req->setPost(true);
	req->setPostContent(post_content);

	req->setRespWriteFunc(cb_func, func_param);
#if !USE_RESPONSE_UNITY
	req->setErrorFunc(err_func, err_param);
#endif

	if (!addReq(req)) {
		freeReq(req);
		if (output_debug_)
			std::cout << "add req failed" << std::endl;
		return -1;
	}

	return 1;
}

int HttpRequestMgr::one_loop()
{
	HttpRequest* req = NULL;
	
	static auto ts = std::chrono::system_clock::now();
	static auto tss = ts;
	static int ndo = 0;
	
	while (true) {
		bool full = processor_.checkMaxProcess();
		if (full) {
			if (output_debug_) {
				auto bs = std::chrono::system_clock::now();
				auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(bs - ts);
				if (duration.count() >= 1000) {
					ts = bs;
					std::cout << "processor is full" << std::endl;
				}
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			break;
		}

		bool has = list_.pop(req); 
		if (!has) {
			if (output_debug_) {
				auto bss = std::chrono::system_clock::now();
				auto duration2 = std::chrono::duration_cast<std::chrono::milliseconds>(bss - tss);
				if (duration2.count() >= 1000) {
					tss = bss;
					std::cout << "list is empty" << std::endl;
				}
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			break;
		}

		if (!processor_.addReq(req)) {
			break;
		}
	}

	int n = processor_.waitResponse(5);
	if (n < 0) {
		if (output_debug_)
			std::cout << "wait failed" << std::endl;
		return -1;
	}

	if (n > 0) {
#if 0
		ndo += n;
		std::cout << "processed " << ndo << std::endl;
		auto es = std::chrono::system_clock::now();
		auto d = std::chrono::duration_cast<std::chrono::milliseconds>(es-ts);
		std::cout << "cost total: " << d.count() << " ms" << std::endl;
#endif
	}
	return 0;
}

int HttpRequestMgr::run()
{
	if (use_thread_) {
		return thread_run();
	} else {
		return one_loop();
	}
}

void HttpRequestMgr::thread_func(void* param)
{
	HttpRequestMgr* mgr = (HttpRequestMgr*)param;
	if (!mgr)
		return;

	mgr->running_ = true;
	
	while (mgr->running_) {
		if (mgr->one_loop() < 0) {
			mgr->running_ = false;
		}
	}
}

int HttpRequestMgr::thread_run()
{
	if (work_thread_ == NULL) {
		work_thread_ = new std::thread(thread_func, this);
	}
	return 0;
}

void HttpRequestMgr::thread_join()
{
	work_thread_->join();
	delete work_thread_;
	work_thread_ = NULL;
}

size_t HttpRequestMgr::write_callback(char* ptr, size_t size, size_t nmemb, void* userdata)
{
	(void)ptr;
	HttpRequest* req = (HttpRequest*)userdata;
	long http_status_code = 0;
	char* url = NULL;	

	req->getResponseCode(&http_status_code);
	req->getPrivate(&url);

	size_t s = size*nmemb;
	return s;
}
