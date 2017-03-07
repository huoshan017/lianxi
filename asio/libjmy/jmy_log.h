#pragma once

#if defined (__cplusplus)
extern "C" {
#endif
#include "thirdparty/include/zlog/zlog.h"
#if defined (__cplusplus)
}
#endif

#include <string>
#include "jmy_singleton.hpp"

class JmyLog : public JmySingleton<JmyLog>
{
public:
	JmyLog();
	~JmyLog();
	bool init(const char* filepath, const char* category);
	bool reload();
	void destroy();
	void log(const char *file, size_t filelen,
			const char *func, size_t funclen,
			long line, int level,
			const char *format, ...);

private:
	zlog_category_t* cat_;
	std::string filepath_;
	std::string category_;
};

#define JmyLogDebug(...) \
	JmyLog::getInstance()->log(__FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, ZLOG_LEVEL_DEBUG, __VA_ARGS__)

#define JmyLogInfo(...)  \
	JmyLog::getInstance()->log(__FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, ZLOG_LEVEL_INFO, __VA_ARGS__)

#define JmyLogNotice(...) \
	JmyLog::getInstance()->log(__FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, ZLOG_LEVEL_NOTICE, __VA_ARGS__)

#define JmyLogWarn(...) \
	JmyLog::getInstance()->log(__FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, ZLOG_LEVEL_WARN, __VA_ARGS__)

#define JmyLogError(...) \
	JmyLog::getInstance()->log(__FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, ZLOG_LEVEL_ERROR, __VA_ARGS__)

#define JmyLogFatal(...) \
	JmyLog::getInstance()->log(__FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, ZLOG_LEVEL_FATAL, __VA_ARGS__)
