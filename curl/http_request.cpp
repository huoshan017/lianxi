#include "http_request.h"
#include <iostream>
using namespace std;

std::unordered_map<HttpRequest*, HttpRequest::resp_cb_data> HttpRequest::eh2func_map_;

HttpRequest::HttpRequest() : eh_(NULL), efunc_(NULL), efunc_param_(NULL), output_debug_(false)
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
	set_default_resp_write_func((void*)this);
	set_default_resp_read_func((void*)this);

	return true;
}

void HttpRequest::close()
{
	if (eh_ == NULL)
		return;

	std::unordered_map<HttpRequest*, resp_cb_data>::iterator it = eh2func_map_.find(this);
	if (it != eh2func_map_.end()) {
		it->second.func = NULL;
		it->second.param = NULL;
	}
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

void HttpRequest::set_resp_func(http_resp_func func, void* userdata)
{
	std::unordered_map<HttpRequest*, resp_cb_data>::iterator it = eh2func_map_.find(this);
	if (it == eh2func_map_.end()) {
		if (output_debug_)
			std::cout << "error: HttpRequest::set_resp_func not found pointer" << std::endl;	
		return;
	}
	it->second.func = func;
	it->second.param = userdata;
}

void HttpRequest::setRespWriteFunc(http_resp_func func, void* userdata)
{
	set_resp_func(func, userdata);
}

void HttpRequest::setRespReadFunc(http_resp_func func, void* userdata)
{
	set_resp_func(func, userdata);
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

void HttpRequest::storeInMap()
{
	resp_cb_data d;
	d.func = NULL;
	d.param = NULL;
	eh2func_map_.insert(std::make_pair(this, d));
}

void HttpRequest::removeFromMap()
{
	eh2func_map_.erase(this);
}

void HttpRequest::set_default_resp_write_func(void* param)
{
	curl_easy_setopt(eh_, CURLOPT_WRITEFUNCTION, default_resp_write_func);
	curl_easy_setopt(eh_, CURLOPT_WRITEDATA, param);
}

void HttpRequest::set_default_resp_read_func(void* param)
{
	curl_easy_setopt(eh_, CURLOPT_READFUNCTION, default_resp_read_func);
	curl_easy_setopt(eh_, CURLOPT_READDATA, param);
}

size_t HttpRequest::default_resp_func(char* ptr, size_t size, size_t nmemb, void* userdata)
{
	HttpRequest* req = (HttpRequest*)userdata;
	std::unordered_map<HttpRequest*, resp_cb_data>::iterator it = eh2func_map_.find(req);
	if (it == eh2func_map_.end()) {
		if (req->output_debug_)
			std::cout << "HttpRequest::default_resp_func handle(" << req << ") cant map to callback function" << std::endl;
	} else {
		long code = 0;
		curl_easy_getinfo(req->getHandle(), CURLINFO_RESPONSE_CODE, &code);
		if (code != 200) {
			if (req->efunc_)	 {
				req->efunc_(code, req->efunc_param_);
			}
		} else {
			it->second.func(ptr, size*nmemb, it->second.param);
		}
	}
	return size*nmemb;
}

size_t HttpRequest::default_resp_write_func(char* ptr, size_t size, size_t nmemb, void* userdata)
{
	return default_resp_func(ptr, size, nmemb, userdata); 
}

size_t HttpRequest::default_resp_read_func(char* ptr, size_t size, size_t nmemb, void* userdata)
{
	return default_resp_func(ptr, size, nmemb, userdata);
}

int HttpRequest::call_error_func(int error)
{
	int res = 0;
	if (efunc_) {
		res = efunc_(error, efunc_param_);
	}
	return res;
}
