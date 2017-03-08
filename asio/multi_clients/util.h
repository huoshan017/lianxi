#pragma once

#include "../libjmy/jmy_log.h"
#include "config_data.h"

#define ClientLogDebug(...) \
	JmyLog::getInstance()->log(s_client_log_cate, __FILE__, sizeof(__FILE__)-1, \
			__func__, sizeof(__func__)-1, __LINE__, ZLOG_LEVEL_DEBUG, __VA_ARGS__)

#define ClientLogInfo(...) \
	JmyLog::getInstance()->log(s_client_log_cate, __FILE__, sizeof(__FILE__)-1, \
			__func__, sizeof(__func__)-1, __LINE__, ZLOG_LEVEL_INFO, __VA_ARGS__)

#define ClientLogNotice(...) \
	JmyLog::getInstance()->log(s_client_log_cate, __FILE__, sizeof(__FILE__)-1, \
			__func__, sizeof(__func__)-1, __LINE__, ZLOG_LEVEL_NOTICE, __VA_ARGS__)

#define ClientLogWarn(...) \
	JmyLog::getInstance()->log(s_client_log_cate, __FILE__, sizeof(__FILE__)-1, \
			__func__, sizeof(__func__)-1, __LINE__, ZLOG_LEVEL_WARN, __VA_ARGS__)

#define ClientLogError(...) \
	JmyLog::getInstance()->log(s_client_log_cate, __FILE__, sizeof(__FILE__)-1, \
			__func__, sizeof(__func__)-1, __LINE__, ZLOG_LEVEL_ERROR, __VA_ARGS__)

#define ClientLogFatal(...) \
	JmyLog::getInstance()->log(s_client_log_cate, __FILE__, sizeof(__FILE__)-1, \
			__func__, sizeof(__func__)-1, __LINE__, ZLOG_LEVEL_FATAL, __VA_ARGS__)
