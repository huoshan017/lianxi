#pragma once

#if defined (__cplusplus)
extern "C" {
#endif
#include "thirdparty/include/zlog/zlog.h"
#if defined (__cplusplus)
}
#endif

#include <unordered_map>
#include <string>
#include "jmy_singleton.hpp"

class JmyLog : public JmySingleton<JmyLog>
{
public:
	JmyLog();
	~JmyLog();
	bool init(const char* filepath);
	bool open(const char* category);
	bool reload();
	void destroy();
	void log(const char* cate_name,
			const char *file, size_t filelen,
			const char *func, size_t funclen,
			long line, int level,
			const char *format, ...);

private:
	std::string filepath_;
	std::unordered_map<std::string, zlog_category_t*> str2cate_;
};

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
