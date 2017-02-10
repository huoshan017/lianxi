#include "http_request.h"
#include <iostream>
using namespace std;

HttpRequest::HttpRequest() : eh_(NULL)
{
}

HttpRequest::~HttpRequest()
{
}

bool HttpRequest::init()
{
	eh_ = curl_easy_init();
	if (eh_ == NULL) {
		return false;
	}
	curl_easy_setopt(eh_, CURLOPT_HEADER, 0L);
	curl_easy_setopt(eh_, CURLOPT_VERBOSE, 0L);
	curl_easy_setopt(eh_, CURLOPT_NOSIGNAL, (long)1);
	setRespWriteFunc(HttpRequest::default_resp_write_func, NULL);
	setRespReadFunc(HttpRequest::default_resp_read_func, NULL);

	return true;
}

void HttpRequest::close()
{
	if (eh_ == NULL)
		return;

	curl_easy_cleanup(eh_);
}

void HttpRequest::setGet(bool enable)
{
	if (eh_ == NULL)
		return;

	long e = enable ? 1L : 0L;
	curl_easy_setopt(eh_, CURLOPT_HTTPGET, e);
}

void HttpRequest::setPost(bool enable)
{
	if (eh_ == NULL)
		return;

	long e = enable ? 1L : 0L;
	curl_easy_setopt(eh_, CURLOPT_POST, e);
}

void HttpRequest::setPostContent(const char* content)
{
	if (eh_ == NULL)
		return;

	curl_easy_setopt(eh_, CURLOPT_POSTFIELDS, content);
}

void HttpRequest::setUrl(const char* url)
{
	if (eh_ == NULL)
		return;

	curl_easy_setopt(eh_, CURLOPT_URL, url);
}

void HttpRequest::setPrivate(void* pointer)
{
	if (eh_ == NULL)
		return;

	curl_easy_setopt(eh_, CURLOPT_PRIVATE, pointer);
}

void HttpRequest::setRespWriteFunc(http_resp_func func, void* userdata)
{
	curl_easy_setopt(eh_, CURLOPT_WRITEFUNCTION, func);
	curl_easy_setopt(eh_, CURLOPT_WRITEDATA, userdata);
}

void HttpRequest::setRespReadFunc(http_resp_func func, void* userdata)
{
	curl_easy_setopt(eh_, CURLOPT_READFUNCTION, func);
	curl_easy_setopt(eh_, CURLOPT_READDATA, userdata);
}

void HttpRequest::setErrorFunc(http_error_func func, void* param)
{
	efunc_  = func;
	efunc_param_ = param;
}

void HttpRequest::getPrivate(char** pri)
{
	curl_easy_getinfo(eh_, CURLINFO_PRIVATE, pri);
}

void HttpRequest::getResponseCode(long* code)
{
	curl_easy_getinfo(eh_, CURLINFO_RESPONSE_CODE, code);
}

size_t HttpRequest::default_resp_write_func(char* ptr, size_t size, size_t nmemb, void* userdata)
{
	(void)ptr;
	(void)userdata;
	return size*nmemb;
}

size_t HttpRequest::default_resp_read_func(char* ptr, size_t size, size_t nmemb, void* userdata)
{
	(void)ptr;
	(void)userdata;
	return size*nmemb;
}

int HttpRequest::call_error_func(int error)
{
	int res = 0;
	if (efunc_) {
		res = efunc_(error, efunc_param_);
	}
	return res;
}
