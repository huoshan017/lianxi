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
template <typename int_type>
class JmyIdGenerator 
{
public:
	JmyIdGenerator() : curr_id_(0) {}
	~JmyIdGenerator() {}
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
			curr_id_ += 1;
			if (used_set_.find(curr_id_) != used_set_.end())
				return 0;
			return curr_id_;
		}
	}
	bool free(int_type id) {
		if (used_set_.find(id) == used_set_.end()) {
			return false;
		}
		used_set_.erase(id);
		free_list_.push_back(id);
		return true;
	}

private:
	int_type curr_id_;
	std::list<int_type> free_list_;
	std::set<int_type> used_set_;
};
