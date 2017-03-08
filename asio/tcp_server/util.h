#pragma once

#include "../libjmy/jmy_log.h"
#include "config_data.h"

#define ServerLogDebug(...) \
	JmyLog::getInstance()->log(s_server_log_cate, __FILE__, sizeof(__FILE__)-1, \
			__func__, sizeof(__func__)-1, __LINE__, ZLOG_LEVEL_DEBUG, __VA_ARGS__)

#define ServerLogInfo(...) \
	JmyLog::getInstance()->log(s_server_log_cate, __FILE__, sizeof(__FILE__)-1, \
			__func__, sizeof(__func__)-1, __LINE__, ZLOG_LEVEL_INFO, __VA_ARGS__)

#define ServerLogNotice(...) \
	JmyLog::getInstance()->log(s_server_log_cate, __FILE__, sizeof(__FILE__)-1, \
			__func__, sizeof(__func__)-1, __LINE__, ZLOG_LEVEL_NOTICE, __VA_ARGS__)

#define ServerLogWarn(...) \
	JmyLog::getInstance()->log(s_server_log_cate, __FILE__, sizeof(__FILE__)-1, \
			__func__, sizeof(__func__)-1, __LINE__, ZLOG_LEVEL_WARN, __VA_ARGS__)

#define ServerLogError(...) \
	JmyLog::getInstance()->log(s_server_log_cate, __FILE__, sizeof(__FILE__)-1, \
			__func__, sizeof(__func__)-1, __LINE__, ZLOG_LEVEL_ERROR, __VA_ARGS__)

#define ServerLogFatal(...) \
	JmyLog::getInstance()->log(s_server_log_cate, __FILE__, sizeof(__FILE__)-1, \
			__func__, sizeof(__func__)-1, __LINE__, ZLOG_LEVEL_FATAL, __VA_ARGS__)
