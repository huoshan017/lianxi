#include "http_request_mgr.h"
#include <thread>
#ifndef WIN32
#include <unistd.h>
#else
#include <windows.h>
#endif
#include <iostream>
#include <memory.h>

const static int DEFAULT_MAX_REQUEST_COUNT = 5000;

HttpRequestMgr* HttpRequestMgr::instance_ = NULL;

HttpRequestMgr::HttpRequestMgr() : running_(false), work_thread_(NULL), use_thread_(false)
{
}

HttpRequestMgr::~HttpRequestMgr()
{
	close();
}

HttpRequestMgr* HttpRequestMgr::getInstance()
{
	if (instance_ == NULL) {
		instance_ = new HttpRequestMgr;
	}
	return instance_;
}

bool HttpRequestMgr::init(int max_nreq)
{
	if (max_nreq <= 0) {
		max_nreq = DEFAULT_MAX_REQUEST_COUNT;
	}

	if (!processor_.init())
		return false;

	list_.init(max_nreq);

	if (!pool_.init(max_nreq))
		return false;

	return true;
}

void HttpRequestMgr::close()
{
	pool_.clear();
	processor_.close();
	//results_.clear();
	running_ = false;
	if (use_thread_) {
		thread_join();
	}
	use_thread_ = false;
}

bool HttpRequestMgr::hasFreeReq()
{
	return pool_.hasFree();
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

#if 0
bool HttpRequestMgr::getResult(HttpResult*& result)
{
	return results_.popResult(result);
}

bool HttpRequestMgr::freeResult(HttpResult* result)
{
	return results_.freeResult(result);
}
#endif

int HttpRequestMgr::one_loop()
{
	HttpRequest* req = NULL;

#ifndef WIN32
	struct timeval tv;
	gettimeofday(&tv, NULL);
	static uint32_t s = (uint32_t)(tv.tv_sec * 1000 + tv.tv_usec/1000);
	
	static time_t t = time(NULL);
	static time_t tt = time(NULL);
#endif
	static int ndo = 0;
	
	while (true) {
		bool full = processor_.checkMaxProcess();
		if (full) {
			time_t b = time(NULL);
#ifndef  WIN32
			if (b - t >= 1) {
				t = b;
				//std::cout << "processor is full" << std::endl;
			}
			usleep(100);
#else
			::Sleep(1);
#endif
			break;
		}

		bool has = list_.pop(req); 
		if (!has) {
			time_t c = time(NULL);
#ifndef WIN32
			if (c - tt >= 1) {
				tt = c;
				//std::cout << "list is empty" << std::endl;
			}
			usleep(100);
#else
			::Sleep(1);
#endif
			break;
		}

		if (!processor_.addReq(req)) {
			break;
		}
	}

	int n = processor_.waitResponse(5);
	if (n < 0) {
		std::cout << "wait failed" << std::endl;
		return -1;
	}

	//results_.doLoop();

	if (n > 0) {
		ndo += n;
#ifndef WIN32
		std::cout << "processed " << ndo << std::endl;
		gettimeofday(&tv, NULL);
		uint32_t e = (uint32_t)(tv.tv_sec*1000+tv.tv_usec/1000);
		std::cout << "cost total: " << e - s << " ms" << std::endl;
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
	HttpRequest* req = (HttpRequest*)userdata;
	long http_status_code = 0;
	char* url = NULL;	

	req->getResponseCode(&http_status_code);
	//req->getPrivate(&url);

	if (http_status_code == 200) {
		//std::cout << "200 OK for " << url << std::endl; 
	} else {
		std::cout << "GET of " <<  url << " returned http status code " << http_status_code << std::endl;
	}

	size_t s = size*nmemb;
	
	/*if (!HttpRequestMgr::getInstance()->results_.insertResult(ptr, s, req)) {
		std::cout << "insert result ptr(" << ptr << "), s(" << s << "), req(" << req << ") failed" << std::endl;
	}*/
	return s;
}
