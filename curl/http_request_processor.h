#pragma once

#ifdef WIN32
#include <windows.h>
#endif
#include "curl/multi.h"
#include "curl/curl.h"

#include <map>

static const int DEFAULT_MAX_PROCESS_COUNT = 2000;

class HttpRequest;

class HttpRequestProcessor
{
public:
	HttpRequestProcessor();
	~HttpRequestProcessor();
	
	bool init(int max_request = DEFAULT_MAX_PROCESS_COUNT);
	void close();

	bool checkMaxProcess() const;
	bool isEmpty() const;

	bool addReq(HttpRequest*);
	bool removeReq(CURL*);
	int waitResponse(int);


private:
	CURLM* handle_;
	int curr_nprocess_; // 当前处理数量
	int max_nprocess_; // 最大处理数量

	int total_nmsg_;	// 已处理消息数
	int total_nmsg_failed_; // 失败的消息数
	std::map<CURL*, HttpRequest*> reqs_map_;
};
