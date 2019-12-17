#include "http_request_processor.h"
#include "http_request.h"
#include "http_request_mgr.h"
#include <iostream>

HttpRequestProcessor::HttpRequestProcessor() : mgr_(nullptr)
{
	handle_ = nullptr;
	max_nprocess_ = 0;
	curr_nprocess_ = 0;
	do_statistics_ = false;
	msg_processed_ = 0;
	msg_failed_ = 0;
	msg_last_processed_ = 0;
	msg_last_failed_ = 0;
}

HttpRequestProcessor::~HttpRequestProcessor()
{
}

bool HttpRequestProcessor::init(int max_nprocess)
{
	curl_global_init(CURL_GLOBAL_ALL);
	handle_ = curl_multi_init();
	if (handle_ == nullptr)
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

bool HttpRequestProcessor::checkMaxProcess() const
{ 
	return (max_nprocess_ <= curr_nprocess_); 
}

bool HttpRequestProcessor::isEmpty() const
{ return curr_nprocess_ <= 0; }

bool HttpRequestProcessor::addReq(HttpRequest* req)
{
	if (handle_ == nullptr)
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
	curr_nprocess_ += 1;

	return true;
}

bool HttpRequestProcessor::removeReq(CURL* req)
{
	if (req == nullptr)
		return false;

	std::map<CURL*, HttpRequest*>::iterator it = reqs_map_.find(req);
	if (it == reqs_map_.end()) {
		return false;
	}

	CURLMcode code = curl_multi_remove_handle(handle_, req);
	if (code != CURLM_OK)
		return false;

	reqs_map_.erase(req);
	if (curr_nprocess_ > 0)
		curr_nprocess_ -= 1;

	return true;
}

int HttpRequestProcessor::waitResponse(int max_wait_msecs)
{
	if (!handle_)
		return -1;

	CURL* eh = nullptr;
	CURLMsg* msg = nullptr;
	CURLcode code;
	int old_still_running = 1;
	int new_still_running = 2;
	int msgs_left = 0;
	CURLMcode mcode;
	HttpRequest* req = nullptr;

	curl_multi_perform(handle_, &old_still_running);
	new_still_running = old_still_running;
	int numfds = 0;
	while (new_still_running >= old_still_running) {
		/* wait for activity, timeout or "nothing" */
		mcode = curl_multi_wait(handle_, nullptr, 0, max_wait_msecs, &numfds);
		if (mcode != CURLM_OK) {
#if 0
			std::cout << "error: curl_multi_wait() return " << mcode << std::endl;
#endif
			return -1;
		}
#if 0
		std::cout << "curl_multi_wait() numfds=" << numfds << ", still_running=" << new_still_running << std::endl;
#endif
		
		if (!numfds)
			break;
		
		curl_multi_perform(handle_, &new_still_running);
	}

	int nmsg_done = 0;
	if (numfds > 0) {
		while ((msg = curl_multi_info_read(handle_, &msgs_left))) {
			if (do_statistics_) {
				if (msg_processed_ == 0) {
					statistic_start_ = std::chrono::system_clock::now();
					qps_last_ = statistic_start_;
				}
				msg_processed_ += 1;
			}

			eh = msg->easy_handle;
			std::map<CURL*, HttpRequest*>::iterator it = reqs_map_.find(eh);
			req = it->second;
			if (it != reqs_map_.end()) {
				if (msg->msg == CURLMSG_DONE) {
					code = msg->data.result;
					if (code != CURLE_OK) {
						req->call_error_func(code);
						if (do_statistics_) {
							msg_failed_ += 1;
							msg_last_failed_ += 1;
							std::cout << "total_failed: " << msg_failed_ << std::endl;
						}
					} else {
						nmsg_done += 1;
					}
				} else {
					req->call_error_func(-1);
					if (do_statistics_) {
						msg_failed_ += 1;
						msg_last_failed_ += 1;
						std::cout << "error: after curl_multi_info_read(), CURLMsg = " << msg->msg << ", " << std::endl;
					}
				}
				if (!removeReq(eh)) {
					if (do_statistics_) {
						msg_failed_ += 1;
						msg_last_failed_ += 1;
					}
					std::cout << "error: remove handle " << eh << " failed" << std::endl;
				}
				if (mgr_)
					mgr_->freeReq(req);
			} else {
				if (do_statistics_) {
					msg_failed_ += 1;
					msg_last_failed_ += 1;
				}
				std::cout << "error: find handle " << eh << " from reqs_map_ failed" << std::endl;
			}
		}
	}

	if (do_statistics_ && msg_processed_ > 0) {
		auto now = std::chrono::system_clock::now();
		auto cost_time = std::chrono::duration_cast<std::chrono::milliseconds>(now-qps_last_).count();
		auto total_cost_time = std::chrono::duration_cast<std::chrono::milliseconds>(now-statistic_start_).count();
		if (cost_time >= 1000) {
			std::cout << "qps: " << (msg_processed_-msg_last_processed_)/(cost_time/1000) << ", qps failed: " << (msg_failed_-msg_last_failed_)/(cost_time/1000) << std::endl;
			std::cout << "total processed: " << msg_processed_ << ", last processed: " << msg_processed_-msg_last_processed_ << ", total failed: " << msg_failed_ << ", last failed: " << msg_failed_-msg_last_failed_ << std::endl;
			std::cout << "cost time(ms): " << cost_time << ", total cost time(ms): " << total_cost_time << std::endl;
			msg_last_processed_ = msg_processed_;
			msg_last_failed_ = msg_failed_;
			qps_last_ = now;
		}
	}
	return nmsg_done;
}
