#include "http_request_mgr.h"
#include "http_request_list.h"
#include <thread>
#include <unistd.h>
#include <iostream>

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

	if (!pool_.init(max_nreq))
		return false;

	if (!processor_.init())
		return false;

	HttpRequestList::getInstance()->init(max_nreq);

	return true;
}

void HttpRequestMgr::close()
{
	pool_.clear();
	processor_.close();
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
	return HttpRequestList::getInstance()->push(req);
}

void HttpRequestMgr::freeReq(HttpRequest* req)
{
	req->close();
	pool_.free(req);
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
				sleep(1);
				break;
			}

			bool has = HttpRequestList::getInstance()->pop(req);
			if (!has) {
				//std::cout << "list is empty" << std::endl;
				sleep(5);
				break;
			}

			//std::cout << "pop one req: " << req << std::endl;
			if (!mgr->processor_.addReq(req)) {
				break;
			}
		}

		int n = mgr->processor_.waitResponse(100);
		if (n < 0)
			mgr->running_ = false;

		ndo += n;
		std::cout << "processed " << ndo << std::endl;

		if (n > 0) {
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
		std::cout << "200 OK for " << url << std::endl; 
	} else {
		//std::cout << "GET of " <<  url << " returned http status code " << http_status_code << std::endl;
	}

	size_t s = size*nmemb;
	//std::cout << "do write_callback: " << ptr << ", " << s << std::endl;
	return s;
}
