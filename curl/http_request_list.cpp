#include "http_request_list.h"

HttpRequestList* HttpRequestList::instance_ = NULL;

HttpRequestList::HttpRequestList() : reqs_(0)
{
}

HttpRequestList::~HttpRequestList()
{
	if (instance_) {
		delete instance_;
		instance_ = NULL;
	}
}

HttpRequestList::HttpRequestList(const HttpRequestList&)
{
}

HttpRequestList* HttpRequestList::getInstance()
{
	if (instance_ == NULL) {
		instance_ = new HttpRequestList;
	}
	return instance_;
}

void HttpRequestList::init(int max_size)
{
	reqs_.reserve(max_size);
}

bool HttpRequestList::push(HttpRequest* req)
{
	return reqs_.push(req);
}

bool HttpRequestList::pop(HttpRequest*& req)
{
	return reqs_.pop(req);
}
