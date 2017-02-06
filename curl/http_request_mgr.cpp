#include "http_request_mgr.h"
#include "http_request_list.h"
#include <thread>
#include <unistd.h>
#include <iostream>
#include <memory.h>

const static int DEFAULT_MAX_REQUEST_COUNT = 5000;

HttpRequestMgr* HttpRequestMgr::instance_ = NULL;

HttpRequestMgr::HttpRequestMgr() : running_(false)
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
	req->setRespWriteFunc(write_callback, (void*)req);
	return list_.push(req);
}

void HttpRequestMgr::freeReq(HttpRequest* req)
{
	req->close();
	pool_.free(req);
}

bool HttpRequestMgr::getResult(char*& p, int& len)
{
	return results_.popResult(p, len);
}

bool HttpRequestMgr::freeResult(char* ptr, int len)
{
	return results_.freeResult(ptr, len);
}

int HttpRequestMgr::run()
{
	std::thread work_thread(thread_func, this);
	work_thread.join();
	return 0;
}

void HttpRequestMgr::thread_func(void* param)
{
	HttpRequestMgr* mgr = (HttpRequestMgr*)param;
	if (!mgr)
		return;

	mgr->running_ = true;
	HttpRequest* req = NULL;

	struct timeval tv;
	gettimeofday(&tv, NULL);
	uint32_t s = (uint32_t)(tv.tv_sec * 1000 + tv.tv_usec/1000);
	
	int ndo = 0;
	while (mgr->running_) {
		while (true) {
			bool full = mgr->processor_.checkMaxProcess();
			if (full) {
				//std::cout << "processor is full" << std::endl;
				usleep(100);
				break;
			}

			bool has = mgr->list_.pop(req); 
			if (!has) {
				//std::cout << "list is empty" << std::endl;
				usleep(100);
				break;
			}

			if (!mgr->processor_.addReq(req)) {
				break;
			}
		}

		int n = mgr->processor_.waitResponse(5);
		if (n < 0)
			mgr->running_ = false;

		mgr->results_.doLoop();

		if (n > 0) {
			ndo += n;
			std::cout << "processed " << ndo << std::endl;
			gettimeofday(&tv, NULL);
			uint32_t e = (uint32_t)(tv.tv_sec*1000+tv.tv_usec/1000);
			std::cout << "cost total: " << e - s << " ms" << std::endl;
		}
	}
}

size_t HttpRequestMgr::write_callback(char* ptr, size_t size, size_t nmemb, void* userdata)
{
	HttpRequest* req = (HttpRequest*)userdata;
	// get HTTP status code
	long http_status_code = 0;
	char* url = NULL;	

	req->getResponseCode(&http_status_code);
	req->getPrivate(&url);

	if (http_status_code == 200) {
		//std::cout << "200 OK for " << url << std::endl; 
	} else {
		std::cout << "GET of " <<  url << " returned http status code " << http_status_code << std::endl;
	}

	size_t s = size*nmemb;
	//std::cout << "do write_callback: " << ptr << ", " << s << std::endl;
	
	HttpRequestMgr::getInstance()->results_.insertResult(ptr, s);
	return s;
}
