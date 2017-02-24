#pragma once
#include <set>
#include <list>
#include <string>
#include <zscommon/json/json.h>
#include "http_request_mgr.h"

struct PostLogRecord {
	std::string operation;
	Json::Value value;
};

struct PostLogData {
	std::string key;
	std::string value;
};

class HttpPostLogMgr
{
public:
	~HttpPostLogMgr();
	static HttpPostLogMgr* getInstance();

	bool init(int max_log_size);
	void clear();

	void setUrl(const char* url);
	void setPostCountOnce(uint16_t count);
	void pushLogData(const char* key, const char* value);
	void pushLogDataAsString(const char* key, int value);
	void pushLogData(const char* key, int value);
	void pushLogData(const char* key, const std::string& value);
	void pushLogData(const char* key, time_t value);
	void begin();
	void end(const std::string&);
	int run();

public:
	class CmpPostLogData {
	public:
		bool operator()(const PostLogData& d1, const PostLogData& d2) const {
			return d1.key < d2.key;
		}
	};
private:
	HttpPostLogMgr();
	HttpPostLogMgr(const HttpPostLogMgr&);
	HttpPostLogMgr& operator=(const HttpPostLogMgr&);

	bool insertLogData(const char* key, const char* value);
	void pushDate();

	std::set<PostLogData, CmpPostLogData> push_data_;
	std::list<PostLogRecord> record_list_;
	Json::Value current_json_;
	HttpRequestMgr request_mgr_;
	int max_log_size_;
	std::string post_url_;
	uint16_t post_count_once_;

	static HttpPostLogMgr* instance_;
};

#define POSTLOG_MGR (HttpPostLogMgr::getInstance())
