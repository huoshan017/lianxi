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

	// 初始化时改变
	free_queue_.reserve(max_size);
	HttpRequest* req = NULL;
	for (int i=0; i<max_size; i++) {
		req = req_pool_.malloc();
		free_queue_.push(req);
		req->storeInMap();
	}

	max_size_ = max_size;
	return true;
}

void HttpRequestPool::clear()
{
	HttpRequest* req = NULL;
	while (true) {
		if (free_queue_.empty())
			break;
		if (!free_queue_.pop(req))
			break;
		req_pool_.destroy(req);
		req->removeFromMap();
	}
}

// call in only one thread
HttpRequest* HttpRequestPool::malloc()
{
	HttpRequest* req = NULL;
	if (!free_queue_.pop(req))
		return NULL;

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
