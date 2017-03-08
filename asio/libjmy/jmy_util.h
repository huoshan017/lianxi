#pragma once
#include "jmy_log.h"

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
