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
	total_vec_.resize(max_size);
	free_queue_.reserve(max_size);
	HttpRequest* req = NULL;
	for (int i=0; i<max_size; i++) {
		req = req_pool_.malloc();
		free_queue_.push(req);
		//total_vec_[i] = std::make_pair(req, false);
		//ptr2idx_map_.insert(std::make_pair(req, i));
	}

	max_size_ = max_size;
	return true;
}

void HttpRequestPool::clear()
{
	HttpRequest* req = NULL;
	std::vector<std::pair<HttpRequest*, bool> >::iterator it = total_vec_.begin();
	for (; it!=total_vec_.end(); ++it) {
		req = it->first;
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

	/*std::unordered_map<HttpRequest*, int>::iterator it = ptr2idx_map_.find(req);
	if (it != ptr2idx_map_.end()) {
		int idx = it->second;
		if (idx>=0 && idx<max_size_) {
			total_vec_[idx].second = true;
		}
	}*/
	return req;
}

bool HttpRequestPool::free(HttpRequest* req)
{
	/*std::unordered_map<HttpRequest*, int>::iterator it = ptr2idx_map_.find(req);
	if (it != ptr2idx_map_.end()) {
		int idx = it->second;
		if (idx>=0 && idx<max_size_) {
			total_vec_[idx].second = false;
		}
	}*/
	return free_queue_.push(req);
}

bool HttpRequestPool::hasFree()
{
	return !free_queue_.empty();
}
