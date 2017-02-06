#include "http_request_processor.h"
#include "http_request_mgr.h"
#include "http_request.h"
#include <iostream>

HttpRequestProcessor::HttpRequestProcessor() :
	handle_(NULL), max_nprocess_(0), cur_nprocess_(0), total_nmsg_(0), total_nmsg_failed_(0)
{
}

HttpRequestProcessor::~HttpRequestProcessor()
{
}

bool HttpRequestProcessor::init(int max_nprocess)
{
	curl_global_init(CURL_GLOBAL_ALL);
	handle_ = curl_multi_init();
	if (handle_ == NULL)
		return false;

	if (max_nprocess <= 0)
		max_nprocess_ = DEFAULT_MAX_PROCESS_COUNT;
	else
		max_nprocess_ = max_nprocess;

	return true;
}

void HttpRequestProcessor::close()
{
	if (!handle_)
		return;

	curl_multi_cleanup(handle_);
	curl_global_cleanup();
}

bool HttpRequestProcessor::addReq(HttpRequest* req)
{
	if (handle_ == NULL)
		return false;

	if (reqs_map_.find(req) != reqs_map_.end())
		return false;

	if (checkMaxProcess()) {
		return false;
	}

	CURLMcode code = curl_multi_add_handle(handle_, req->getHandle());
	if (code != CURLM_OK)
		return false;

	reqs_map_.insert(std::make_pair(req->getHandle(), req));
	cur_nprocess_ += 1;

	return true;
}

bool HttpRequestProcessor::removeReq(CURL* req)
{
	if (req == NULL)
		return false;

	std::map<CURL*, HttpRequest*>::iterator it = reqs_map_.find(req);
	if (it == reqs_map_.end()) {
		return false;
	}

	CURLMcode code = curl_multi_remove_handle(handle_, req);
	if (code != CURLM_OK)
		return false;

	reqs_map_.erase(req);
	if (cur_nprocess_ > 0)
		cur_nprocess_ -= 1;

	return true;
}

int HttpRequestProcessor::waitResponse(int max_wait_msecs)
{
	if (!handle_)
		return -1;

	CURL* eh = NULL;
	CURLMsg* msg = NULL;
	CURLcode code;
	int still_running = 1;
	int msgs_left = 0;
	CURLMcode mcode;
	HttpRequest* req = NULL;

	curl_multi_perform(handle_, &still_running);

	while (still_running) {
		if (!still_running) {
			std::cout << "no still running" << std::endl;
			return 0;
		}

		/* wait for activity, timeout or "nothing" */
		int numfds = 0;
		mcode = curl_multi_wait(handle_, NULL, 0, max_wait_msecs, &numfds);
		if (mcode != CURLM_OK) {
			std::cout << "error: curl_multi_wait() return " << mcode << std::endl;
			return -1;
		}

		if (!numfds) {
			//std::cout << "curl_multi_wait() numfds=" << numfds << std::endl;
			return 0;
		}
		curl_multi_perform(handle_, &still_running);
	}

	int nmsg_done = 0;
	while ((msg = curl_multi_info_read(handle_, &msgs_left))) {
		total_nmsg_ += 1;
		if (msg->msg == CURLMSG_DONE) {
			code = msg->data.result;
			if (code != CURLE_OK) {
				total_nmsg_failed_ += 1;
				std::cout << "CURL error code: " << code << ", total_nmsg_no: " << total_nmsg_ << ", total_failed: " << total_nmsg_failed_ << std::endl;
			} else {
				nmsg_done += 1;
			}
		} else {
			total_nmsg_failed_ += 1;
			std::cout << "error: after curl_multi_info_read(), CURLMsg = " << msg->msg << ", " << std::endl;
		}

		eh = msg->easy_handle;
		std::map<CURL*, HttpRequest*>::iterator it = reqs_map_.find(eh);
		req = it->second;
		if (!removeReq(eh)) {
			std::cout << "error: remove handle " << eh << " failed" << std::endl;
		}
		if (it == reqs_map_.end()) {
			std::cout << "error: find handle " << eh << " from reqs_map_ failed" << std::endl;
		} else {
			HttpRequestMgr::getInstance()->freeReq(req);
		}
	}
	return nmsg_done;
}
