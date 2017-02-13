#include "http_request_results.h"
#include "http_request_mgr.h"
#include <string.h>
#include <iostream>

HttpRequestResults::HttpRequestResults() 
{
}

HttpRequestResults::~HttpRequestResults()
{
}

void HttpRequestResults::init(int size)
{
	//results_pool_.init(size);
	results_.init(size);
	to_free_results_.init(size);
}

void HttpRequestResults::deallocResults()
{
	HttpResult* res = NULL;
	while (true) {
		if (!results_.pop(res))
			break;
	}
}

void HttpRequestResults::deallocToFreeResults()
{
	HttpResult* res = NULL;
	while (true) {
		if (!to_free_results_.pop(res))
			break;
	}
}

void HttpRequestResults::clear()
{
	deallocResults();
	deallocToFreeResults();
	//results_pool_.clear();
}

bool HttpRequestResults::insertResult(char* str, int len, HttpRequest* req)
{
	if (str == NULL) return false;
	HttpResult* r = NULL; //results_pool_.malloc();
	if (r == NULL) {
		std::cout << "results_pool_ malloc failed" << std::endl;
		return false;
	}

	r->str = str;
	r->len = len;
	r->req = req;
	return results_.push(r);
}

bool HttpRequestResults::popResult(HttpResult*& res)
{
	if (!results_.pop(res))
		return false;
	
	return true;
}

bool HttpRequestResults::freeResult(HttpResult* res)
{
	return to_free_results_.push(res);
}

void HttpRequestResults::doLoop()
{
	HttpResult* res = NULL;
	while (true) {
		if (!to_free_results_.pop(res))
			break;

		//HttpRequestMgr::getInstance()->freeReq(res->req);
		//results_pool_.free(res);
	}
}
