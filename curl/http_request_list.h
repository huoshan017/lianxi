#pragma once

#include <boost/lockfree/queue.hpp>

class HttpRequest;

class HttpRequestList
{
public:
	~HttpRequestList();
	static HttpRequestList* getInstance();

	void init(int max_size);
	bool push(HttpRequest*);
	bool pop(HttpRequest*&);

private:
	HttpRequestList();
	HttpRequestList(const HttpRequestList& );
	HttpRequestList& operator = (const HttpRequestList&);

	boost::lockfree::queue<HttpRequest*> reqs_;
	static HttpRequestList* instance_;
};
