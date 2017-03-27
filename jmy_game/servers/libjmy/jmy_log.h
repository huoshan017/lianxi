#pragma once

#if defined (__cplusplus)
extern "C" {
#endif
#include "thirdparty/include/zlog/zlog.h"
#if defined (__cplusplus)
}
#endif

//#include <unordered_map>
#include <string>
#include "jmy_singleton.hpp"

class JmyLog : public JmySingleton<JmyLog>
{
public:
	JmyLog();
	~JmyLog();
	bool init(const char* filepath);
	bool open(const char* category);
	bool openLib(const char* category);
	bool reload();
	void close();
	void log(const char* cate_name,
			const char *file, size_t filelen,
			const char *func, size_t funclen,
			long line, int level,
			const char *format, ...);
	void logLib(const char *file, size_t filelen,
			const char *func, size_t funclen,
			long line, int level,
			const char *format, ...);

private:
	std::string filepath_;
	//std::unordered_map<std::string, zlog_category_t*> str2cate_;
	struct cate_pair {
		std::string cate_str;
		zlog_category_t* cate;
	};
	enum { MaxCateCount = 10 };
	cate_pair cates_[MaxCateCount];	
	std::string lib_str_;
	zlog_category_t* lib_cate_;
};

#define JmyLogInit(config_file) \
	JmyLog::getInstance()->init(config_file)

#define JmyLogOpen(cate_name) \
	JmyLog::getInstance()->open(cate_name)

#define JmyLogOpenLib(cate_name) \
	JmyLog::getInstance()->openLib(cate_name)

#define LibJmyLogDebug(...) \
	JmyLog::getInstance()->logLib(__FILE__, sizeof(__FILE__)-1, \
			__func__, sizeof(__func__)-1, __LINE__, ZLOG_LEVEL_DEBUG, __VA_ARGS__)

#define LibJmyLogInfo(...) \
	JmyLog::getInstance()->logLib(__FILE__, sizeof(__FILE__)-1, \
			__func__, sizeof(__func__)-1, __LINE__, ZLOG_LEVEL_INFO, __VA_ARGS__)

#define LibJmyLogNotice(...) \
	JmyLog::getInstance()->logLib(__FILE__, sizeof(__FILE__)-1, \
			__func__, sizeof(__func__)-1, __LINE__, ZLOG_LEVEL_NOTICE, __VA_ARGS__)

#define LibJmyLogWarn(...) \
	JmyLog::getInstance()->logLib(__FILE__, sizeof(__FILE__)-1, \
			__func__, sizeof(__func__)-1, __LINE__, ZLOG_LEVEL_WARN, __VA_ARGS__)

#define LibJmyLogError(...) \
	JmyLog::getInstance()->logLib(__FILE__, sizeof(__FILE__)-1, \
			__func__, sizeof(__func__)-1, __LINE__, ZLOG_LEVEL_ERROR, __VA_ARGS__)

#define LibJmyLogFatal(...) \
	JmyLog::getInstance()->logLib(__FILE__, sizeof(__FILE__)-1, \
			__func__, sizeof(__func__)-1, __LINE__, ZLOG_LEVEL_FATAL, __VA_ARGS__)
