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

void HttpRequest::setRespWriteFunc(resp_func func, void* userdata)
{
	curl_easy_setopt(eh_, CURLOPT_WRITEFUNCTION, func);
	curl_easy_setopt(eh_, CURLOPT_WRITEDATA, userdata);
}

void HttpRequest::setRespReadFunc(resp_func func, void* userdata)
{
	curl_easy_setopt(eh_, CURLOPT_READFUNCTION, func);
	curl_easy_setopt(eh_, CURLOPT_READDATA, userdata);
}

void HttpRequest::getPrivate(char** pri)
{
	curl_easy_getinfo(eh_, CURLINFO_PRIVATE, pri);
}

void HttpRequest::getResponseCode(long* code)
{
	curl_easy_getinfo(eh_, CURLINFO_RESPONSE_CODE, code);
}
