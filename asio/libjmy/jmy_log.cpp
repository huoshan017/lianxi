#include "jmy_log.h"
#include <stdarg.h>

JmyLog::JmyLog() : cat_(NULL)
{
}

JmyLog::~JmyLog()
{
	destroy();
}

bool JmyLog::init(const char* filepath, const char* category)
{
	int rc = zlog_init(filepath);
	if (rc != 0) {
		return false;
	}
	cat_ = zlog_get_category(category);
	if (!cat_) {
		zlog_fini();
		return false;
	}
	filepath_ = filepath;
	category_ = category;
	return true;
}

bool JmyLog::reload()
{
	int rc = zlog_reload(filepath_.c_str());
	if (rc != 0) {
		return false;
	}
	cat_ = zlog_get_category(category_.c_str());
	if (!cat_) {
		zlog_fini();
		return false;
	}
	return true;
}

void JmyLog::destroy()
{
	zlog_fini();
	cat_ = NULL;
}

void JmyLog::log(const char *file, size_t filelen,
				const char *func, size_t funclen,
				long line, int level,
				const char *format, ...)
{
	if (!cat_)
		return;

	va_list vl;
	va_start(vl, format);
	vzlog(cat_, file, filelen, func, funclen, line, level, format, vl);
	va_end(vl);
}
