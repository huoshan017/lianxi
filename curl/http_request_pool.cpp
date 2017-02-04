#include "http_request_pool.h"
#include "http_request.h"

HttpRequestPool::HttpRequestPool() : max_size_(0), free_queue_(max_size_)
{
}

HttpRequestPool::~HttpRequestPool()
{
}

bool HttpRequestPool::init(int max_size)
{
	if (max_size <= 0)
		return false;

	free_queue_.reserve(max_size);
	HttpRequest* req = NULL;
	for (int i=0; i<max_size; i++) {
		req = req_pool_.malloc();
		free_queue_.push(req);
	}

	max_size_ = max_size;
	return true;
}

void HttpRequestPool::clear()
{
	HttpRequest* req = NULL;
	for (; ;) {
		if (!free_queue_.pop(req)) {
			break;
		}
		req_pool_.destroy(req);
	}
	std::list<HttpRequest*>::iterator it = used_queue_.begin();
	for (; it!=used_queue_.end(); ++it) {
		req = *it;
		if (!req) continue;
		req_pool_.destroy(req);
	}
}

// call in only one thread
HttpRequest* HttpRequestPool::malloc()
{
	HttpRequest* req = NULL;
	if (!free_queue_.pop(req))
		return NULL;

	used_queue_.push_back(req);
	return req;
}

bool HttpRequestPool::free(HttpRequest* req)
{
	return free_queue_.push(req);
}

bool HttpRequestPool::hasFree()
{
	return !free_queue_.empty();
}
