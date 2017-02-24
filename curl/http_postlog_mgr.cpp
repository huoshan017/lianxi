#include "http_postlog_mgr.h"
#include <chrono>
#include <ctime>
#include <iostream>
#include <zscommon/util/md5.hpp>
#include "http_request_mgr.h"

static const uint16_t DEFAULT_POST_COUNT_ONCE = 128;

HttpPostLogMgr* HttpPostLogMgr::instance_ = NULL;

HttpPostLogMgr::HttpPostLogMgr() : max_log_size_(0),  post_count_once_(DEFAULT_POST_COUNT_ONCE)
{
}

HttpPostLogMgr::~HttpPostLogMgr()
{
}

HttpPostLogMgr* HttpPostLogMgr::getInstance()
{
	if (!instance_) {
		instance_ = new HttpPostLogMgr();
	}
	return instance_;
}

bool HttpPostLogMgr::init(int max_log_size)
{
	if (max_log_size <= 0)
		max_log_size_ = 5000;
	else
		max_log_size_ = max_log_size;
	bool b = request_mgr_.init(max_log_size_);
	return b;
}

void HttpPostLogMgr::clear()
{
	request_mgr_.close();
}

void HttpPostLogMgr::setUrl(const char* url)
{
	post_url_ = url;
	std::string::size_type s = post_url_.size();
	if (post_url_.at(s-1) != '/') {
		post_url_ += "/";
	}
	std::cout << "HttpPostLogMgr: set url " << post_url_ << std::endl;
}

void HttpPostLogMgr::setPostCountOnce(uint16_t count)
{
	if (count <= 0) {
		count = DEFAULT_POST_COUNT_ONCE;
	} else if (count > max_log_size_) {
		count = max_log_size_;
	}
	post_count_once_ = count;
}

void HttpPostLogMgr::pushLogData(const char* key, const char* value)
{
	if (!key) return;
	current_json_[key] = value;
	insertLogData(key, value);
}

void HttpPostLogMgr::pushLogDataAsString(const char* key, int value)
{
	if (!key) return;
	current_json_[key] = value;
	pushLogData(key, std::to_string(value));
}

void HttpPostLogMgr::pushLogData(const char* key, int value)
{
	if (!key) return;
	current_json_[key] = value;
	insertLogData(key, std::to_string(value).c_str());
}

void HttpPostLogMgr::pushLogData(const char* key, const std::string& value)
{
	if (!key) return;
	current_json_[key] = value;
	insertLogData(key, value.c_str());
}

void HttpPostLogMgr::pushLogData(const char* key, time_t value)
{
	if (!key) return;
	current_json_[key] = (int)value;
	insertLogData(key, std::to_string((int)value).c_str());
}

bool HttpPostLogMgr::insertLogData(const char* key, const char* value)
{
	if (!key || !value) return false;
	PostLogData data;
	data.key = key;
	data.value = value;
	auto it = push_data_.find(data);
	if (it != push_data_.end()) {
		push_data_.erase(it);
	}
	push_data_.insert(data);
	return true;
}

void HttpPostLogMgr::begin()
{
	current_json_.clear();
	push_data_.clear();
	pushDate();
}

void HttpPostLogMgr::end(const std::string& operation)
{
	std::string val_str;
	auto it = push_data_.begin();
	for (; it!=push_data_.end(); ++it) {
		val_str += it->value;
	}
	static char output[4096];
	std::string res = zscommon::util::md5_string((const unsigned char*)val_str.c_str(), output, sizeof(output)-1);
	current_json_["md5data"] = res;

	PostLogRecord record;
	record.operation = operation;
	record.value = current_json_;

	record_list_.push_back(record);
	std::cout << "HttpPostLogMgr::end  push back record(" << operation << ", " << current_json_ << ")" << std::endl;
}

int HttpPostLogMgr::run()
{
	if (post_url_ == "") {
		return 0;
	}

	Json::FastWriter writer;
	int res = 0;
	int16_t c = 0;
	auto it = record_list_.begin();
	for (; it!=record_list_.end();++it) {
		std::cout << "HttpPostLogMgr::run before post" << std::endl;
		std::string url = post_url_ + it->operation;
		res = request_mgr_.post(url.c_str(), writer.write(it->value).c_str());
		if (res < 0) {
			std::cout << "run failed" << std::endl;
			return -1;
		}
		std::cout << "HttpPostLogMgr::run after post" << std::endl;
		
		if (res == 0)
			break;
		else {
			// posted count
			c += 1;
			if (c >= post_count_once_)
				break;
		}
	}

	// remove posted logs 
	while (c > 0) {
		record_list_.pop_front();
		c -= 1;
		std::cout << "pop one front, left " << c << std::endl;
	}
	return res;
}

void HttpPostLogMgr::pushDate()
{
	std::time_t t = std::time(nullptr);
	std::tm* ct = std::localtime(&t);

	pushLogData("log_ymd", ct->tm_year * 10000 + ct->tm_mon * 100 + ct->tm_mday);
	pushLogData("log_ym", ct->tm_year*100 + ct->tm_mon);
	pushLogData("year", ct->tm_year);
	pushLogData("month", ct->tm_mon);
	pushLogData("day", ct->tm_mday);
	pushLogData("hour", ct->tm_hour);
	pushLogData("minute", ct->tm_min);
	pushLogData("week", (ct->tm_yday+6)/7);
}
