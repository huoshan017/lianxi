#pragma once

#include <boost/lockfree/queue.hpp>
#include <boost/pool/object_pool.hpp>
#include <list>

class HttpRequest;

class HttpRequestPool
{
public:
	HttpRequestPool();
	~HttpRequestPool();

	bool init(int max_size);
	void clear();
	HttpRequest* malloc();
	bool free(HttpRequest* req);
	bool hasFree();

private:
	int max_size_;	
	boost::lockfree::queue<HttpRequest*> free_queue_;
	std::list<HttpRequest*> used_queue_;
	boost::object_pool<HttpRequest> req_pool_;
};
