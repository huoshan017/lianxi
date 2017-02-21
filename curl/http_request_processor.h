#pragma once

#ifdef WIN32
#include <windows.h>
#endif
#include "curl/multi.h"
#include "curl/curl.h"
#include <map>
#include <chrono>

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
	void setStatistics(bool enable = false) { do_statistics_ = enable; }

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
	bool do_statistics_;
	int msg_processed_;
	int msg_failed_;
	int msg_last_processed_;
	int msg_last_failed_;
	std::chrono::system_clock::time_point statistic_start_;
	std::chrono::system_clock::time_point qps_last_;
	std::map<CURL*, HttpRequest*> reqs_map_;
};
