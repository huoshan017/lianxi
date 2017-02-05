#pragma once

#include "curl/curl.h"

typedef size_t (*resp_func)(char* ptr, size_t size, size_t nmemb, void* userdata);

class HttpRequest
{
public:
	HttpRequest();
	~HttpRequest();

	bool init();
	void close();
	
	CURL* getHandle() const { return eh_; }

	void setGet(bool enable);
	void setPost(bool enable);
	void setPostContent(const char* content);
	void setUrl(const char* url);
	void setPrivate(void* pointer);
	void setRespWriteFunc(resp_func func, void* userdata);
	void setRespReadFunc(resp_func func, void* userdata);
	void getPrivate(char** pri);
	void getResponseCode(long* code);

private:
	CURL* eh_;
	friend class HttpRequestMgr;
};
