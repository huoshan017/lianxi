#pragma once

#ifdef WIN32
#include <windows.h>
#endif
#include "curl/curl.h"

typedef size_t (*http_resp_func)(char* ptr, size_t size, size_t nmemb, void* userdata);
typedef int (*http_error_func)(int error, void* param);

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
	void setRespWriteFunc(http_resp_func func, void* userdata);
	void setRespReadFunc(http_resp_func func, void* userdata);
	void setErrorFunc(http_error_func func, void* param);
	void getPrivate(char** pri);
	void getResponseCode(long* code);

private:
	int call_error_func(int error);
	static size_t default_resp_write_func(char* ptr, size_t size, size_t nmemb, void* userdata);
	static size_t default_resp_read_func(char* ptr, size_t size, size_t nmemb, void* userdata);

private:
	CURL* eh_;
	http_error_func efunc_;
	void* efunc_param_;
	friend class HttpRequestMgr;
	friend class HttpRequestProcessor;
};
