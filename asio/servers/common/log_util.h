#pragma once

#define LogDebug(cate, ...) \
	JmyLog::getInstance()->log(cate, __FILE__, sizeof(__FILE__)-1, \
			__func__, sizeof(__func__)-1, __LINE__, ZLOG_LEVEL_DEBUG, __VA_ARGS__)

#define LogInfo(cate, ...) \
	JmyLog::getInstance()->log(cate, __FILE__, sizeof(__FILE__)-1, \
			__func__, sizeof(__func__)-1, __LINE__, ZLOG_LEVEL_INFO, __VA_ARGS__)

#define LogNotice(cate, ...) \
	JmyLog::getInstance()->log(cate, __FILE__, sizeof(__FILE__)-1, \
			__func__, sizeof(__func__)-1, __LINE__, ZLOG_LEVEL_NOTICE, __VA_ARGS__)

#define LogWarn(cate, ...) \
	JmyLog::getInstance()->log(cate, __FILE__, sizeof(__FILE__)-1, \
			__func__, sizeof(__func__)-1, __LINE__, ZLOG_LEVEL_WARN, __VA_ARGS__)

#define LogError(cate, ...) \
	JmyLog::getInstance()->log(cate, __FILE__, sizeof(__FILE__)-1, \
			__func__, sizeof(__func__)-1, __LINE__, ZLOG_LEVEL_ERROR, __VA_ARGS__)

#define LogFatal(cate, ...) \
	JmyLog::getInstance()->log(cate, __FILE__, sizeof(__FILE__)-1, \
			__func__, sizeof(__func__)-1, __LINE__, ZLOG_LEVEL_FATAL, __VA_ARGS__)
