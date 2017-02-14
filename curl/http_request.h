#pragma once

#ifdef WIN32
#include <windows.h>
#endif
#include "curl/curl.h"
#include <unordered_map>

typedef void (*http_resp_func)(char* ptr, size_t size, void* userdata);
typedef int (*http_error_func)(int error, void* param);

class HttpRequest
{
public:
	HttpRequest();
	~HttpRequest();

	bool init();
	void close();
	void outputDebug(bool enable = false) { output_debug_ = enable; }
	
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
	void storeInMap();
	void removeFromMap();

private:
	int call_error_func(int error);
	void set_resp_func(http_resp_func, void* userdata);
	void set_default_resp_write_func(void* param);
	void set_default_resp_read_func(void* parm);
	static size_t default_resp_func(char* ptr, size_t size, size_t nmemb, void* param);
	static size_t default_resp_write_func(char* ptr, size_t size, size_t nmemb, void* userdata);
	static size_t default_resp_read_func(char* ptr, size_t size, size_t nmemb, void* userdata);

public:
	struct resp_cb_data {
		http_resp_func func;
		void* param;
	};

private:
	CURL* 				eh_;
	http_error_func 	efunc_;
	void* 				efunc_param_;
	bool				output_debug_;
	static std::unordered_map<HttpRequest*, resp_cb_data> 	eh2func_map_;
	friend class HttpRequestMgr;
	friend class HttpRequestProcessor;
};
