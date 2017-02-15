#pragma once

#ifdef WIN32
#include <windows.h>
#endif
#include "curl/multi.h"
#include "curl/curl.h"
#include <map>

static const int DEFAULT_MAX_PROCESS_COUNT = 2000;

class HttpRequest;
class HttpRequestMgr;

class HttpRequestProcessor
{
public:
	HttpRequestProcessor();
	~HttpRequestProcessor();
	
	void setMgr(HttpRequestMgr* mgr) { mgr_ = mgr; }
	bool init(int max_request = DEFAULT_MAX_PROCESS_COUNT);
	void close();
	void setOutputDebug(bool enable = false) { output_debug_ = enable; }

	bool checkMaxProcess() const;
	bool isEmpty() const;

	bool addReq(HttpRequest*);
	bool removeReq(CURL*);
	int waitResponse(int);


private:
	HttpRequestMgr* mgr_;
	CURLM* handle_;
	int curr_nprocess_;
	int max_nprocess_;
	bool output_debug_;
	int total_nmsg_;	
	int total_nmsg_failed_;
	std::map<CURL*, HttpRequest*> reqs_map_;
};
