#include "http_request_results.h"
#include <string.h>

HttpRequestResults::HttpRequestResults() : results_(0), to_free_results_(0)
{
}

HttpRequestResults::~HttpRequestResults()
{
}

void HttpRequestResults::init(int size)
{
	results_.reserve(size);
	to_free_results_.reserve(size);
}

void HttpRequestResults::deallocResults()
{
	Result res;
	while (true) {
		if (results_.empty())
			break;
		if (!results_.pop(res))
			break;
		results_alloc_.deallocate((char*)res.str, res.len);
	}
}

void HttpRequestResults::deallocToFreeResults()
{
	Result res;
	while (true) {
		if (to_free_results_.empty())
			break;
		if (!to_free_results_.pop(res))
			break;
		results_alloc_.deallocate((char*)res.str, res.len);
	}
}

void HttpRequestResults::clear()
{
	deallocResults();
	deallocToFreeResults();
}

bool HttpRequestResults::insertResult(const char* str, int len)
{
	if (str == NULL) return false;
	char* p = results_alloc_.allocate(len+1);
	memcpy(p, str, len);
	Result res;
	res.str = p;
	res.len = len;
	return results_.push(res);
}

bool HttpRequestResults::popResult(char*& p, int& len)
{
	Result res;
	if (!results_.pop(res))
		return false;
	
	p = (char*)res.str;
	len = res.len;
	return true;
}

bool HttpRequestResults::freeResult(char* str, int len)
{
	Result res;
	res.str = str;
	res.len = len;
	return to_free_results_.push(res);
}

void HttpRequestResults::doLoop()
{
	deallocToFreeResults();
}
