#pragma once

#include "curl/multi.h"
#include "curl/curl.h"
#include <map>

class HttpRequest;

class HttpRequestProcessor
{
public:
	HttpRequestProcessor();
	~HttpRequestProcessor();
	
	bool init(int max_request = 0);
	void close();

	bool checkMaxProcess() const { return (max_nprocess_ <= cur_nprocess_); }
	bool isEmpty() const { return cur_nprocess_ <= 0; }

	bool addReq(HttpRequest*);
	bool removeReq(CURL*);
	int waitResponse(int);

private:
	CURLM* handle_;
	std::map<CURL*, HttpRequest*> reqs_map_;
	int max_nprocess_; // 最大处理数量
	int cur_nprocess_; // 当前处理数量
};
