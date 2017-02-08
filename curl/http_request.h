#pragma once

#ifdef WIN32
#include <windows.h>
#endif
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
	static size_t default_resp_write_func(char* ptr, size_t size, size_t nmemb, void* userdata);
	static size_t default_resp_read_func(char* ptr, size_t size, size_t nmemb, void* userdata);

private:
	CURL* eh_;
	friend class HttpRequestMgr;
};
