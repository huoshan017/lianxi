#pragma once
#include "jmy_log.h"
#include <list>
#include <set>

/* log macros */
#define JmyLogDebug(cate_name, ...) \
	JmyLog::getInstance()->log(cate_name, __FILE__, sizeof(__FILE__)-1,\
			__func__, sizeof(__func__)-1, __LINE__, ZLOG_LEVEL_DEBUG, __VA_ARGS__)

#define JmyLogInfo(cate_name, ...)  \
	JmyLog::getInstance()->log(cate_name, __FILE__, sizeof(__FILE__)-1,\
			__func__, sizeof(__func__)-1, __LINE__, ZLOG_LEVEL_INFO, __VA_ARGS__)

#define JmyLogNotice(cate_name, ...) \
	JmyLog::getInstance()->log(cate_name, __FILE__, sizeof(__FILE__)-1,\
			__func__, sizeof(__func__)-1, __LINE__, ZLOG_LEVEL_NOTICE, __VA_ARGS__)

#define JmyLogWarn(cate_name, ...) \
	JmyLog::getInstance()->log(cate_name, __FILE__, sizeof(__FILE__)-1,\
			__func__, sizeof(__func__)-1, __LINE__, ZLOG_LEVEL_WARN, __VA_ARGS__)

#define JmyLogError(cate_name, ...) \
	JmyLog::getInstance()->log(cate_name, __FILE__, sizeof(__FILE__)-1,\
			__func__, sizeof(__func__)-1, __LINE__, ZLOG_LEVEL_ERROR, __VA_ARGS__)

#define JmyLogFatal(cate_name, ...) \
	JmyLog::getInstance()->log(cate_name, __FILE__, sizeof(__FILE__)-1,\
			__func__, sizeof(__func__)-1, __LINE__, ZLOG_LEVEL_FATAL, __VA_ARGS__)

/* id generator */
template <typename int_type, int_type max_id = 0, int_type min_id = 1>
class JmyIdGenerator 
{
public:
	typedef typename std::enable_if<std::is_integral<int_type>::value, int_type>::type id_type;
	enum { MIN_ID = 1 };

	JmyIdGenerator() : curr_id_(0), max_id_(max_id) {
		if (min_id < MIN_ID) min_id_ = MIN_ID;
		else min_id_ = MIN_ID;
	}
	~JmyIdGenerator() {}
	void init(id_type maxid, id_type minid) {
		min_id_ = minid;
		max_id_ = maxid;
	}
	void reset() {
		curr_id_ = 0;
		free_list_.clear();
		used_set_.clear();
	}
	int_type get() {
		if (free_list_.size() > 0) {
			int_type id = free_list_.front();
			free_list_.pop_front();
			used_set_.insert(id);
			return id;
		} else {
			if (max_id_ > 0) {
				if (curr_id_ >= max_id_) {
					curr_id_ = min_id_;
				} else {
					curr_id_ += 1;
				}
			} else {
				curr_id_ += 1;
			}
			if (curr_id_ <= 0) {
				curr_id_ = min_id_;
			}
			if (used_set_.find(curr_id_) != used_set_.end())
				return 0;
			return curr_id_;
		}
	}
	bool free(id_type id) {
		if (used_set_.find(id) == used_set_.end()) {
			return false;
		}
		used_set_.erase(id);
		free_list_.push_back(id);
		return true;
	}

private:
	id_type curr_id_;
	id_type min_id_;
	id_type max_id_;
	std::list<id_type> free_list_;
	std::set<id_type> used_set_;
};
