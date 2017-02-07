#include "http_request_mgr.h"
#include <thread>
#include <unistd.h>
#include <iostream>
#include <memory.h>

const static int DEFAULT_MAX_REQUEST_COUNT = 5000;

HttpRequestMgr* HttpRequestMgr::instance_ = NULL;

HttpRequestMgr::HttpRequestMgr() : running_(false), work_thread_(NULL)
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

	// HttpRequest对象池
	if (!pool_.init(max_nreq))
		return false;

	// 等待处理列表
	list_.init(max_nreq);

	// 同时处理的最大连接数用默认
	if (!processor_.init())
		return false;

	// 回调结果分配
	results_.init(max_nreq);

	return true;
}

void HttpRequestMgr::close()
{
	pool_.clear();
	processor_.close();
	results_.clear();
	running_ = false;
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
	//req->setRespWriteFunc(write_callback, (void*)req);
	return list_.push(req);
}

void HttpRequestMgr::freeReq(HttpRequest* req)
{
	req->close();
	pool_.free(req);
}

bool HttpRequestMgr::getResult(HttpResult*& result)
{
	return results_.popResult(result);
}

bool HttpRequestMgr::freeResult(HttpResult* result)
{
	return results_.freeResult(result);
}

int HttpRequestMgr::one_loop()
{
	HttpRequest* req = NULL;

	struct timeval tv;
	gettimeofday(&tv, NULL);
	uint32_t s = (uint32_t)(tv.tv_sec * 1000 + tv.tv_usec/1000);
	
	static time_t t = time(NULL);
	static time_t tt = time(NULL);
	static int ndo = 0;
	
	while (true) {
		bool full = processor_.checkMaxProcess();
		if (full) {
			time_t b = time(NULL);
			if (b - t >= 1 ) {
				t = b;
				std::cout << "processor is full" << std::endl;
			}
			usleep(100);
			break;
		}

		bool has = list_.pop(req); 
		if (!has) {
			time_t c = time(NULL);
			if (c - tt >= 1) {
				tt = c;
				std::cout << "list is empty" << std::endl;
			}
			usleep(100);
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

	results_.doLoop();

	if (n > 0) {
		ndo += n;
		std::cout << "processed " << ndo << std::endl;
		gettimeofday(&tv, NULL);
		uint32_t e = (uint32_t)(tv.tv_sec*1000+tv.tv_usec/1000);
		std::cout << "cost total: " << e - s << " ms" << std::endl;
	}
	return 0;
}

void HttpRequestMgr::run()
{
	running_ = true;
	
	while (running_) {
		if (one_loop() < 0) {
			running_ = false;
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
	work_thread_ = NULL;
}

void HttpRequestMgr::thread_func(void* param)
{
	HttpRequestMgr* mgr = (HttpRequestMgr*)param;
	if (!mgr)
		return;

	mgr->run();
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
	
	if (!HttpRequestMgr::getInstance()->results_.insertResult(ptr, s, req)) {
		std::cout << "insert result ptr(" << ptr << "), s(" << s << "), req(" << req << ") failed" << std::endl;
	}
	return s;
}
