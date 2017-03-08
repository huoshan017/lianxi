#include "jmy_log.h"
#include <stdarg.h>

JmyLog::JmyLog()
{
}

JmyLog::~JmyLog()
{
	destroy();
}

bool JmyLog::init(const char* filepath)
{
	int rc = zlog_init(filepath);
	if (rc != 0) {
		return false;
	}
	filepath_ = filepath;
	return true;
}

bool JmyLog::open(const char* category)
{
	zlog_category_t* cat = zlog_get_category(category);
	if (!cat) {
		return false;
	}
	str2cate_.insert(std::make_pair(category, cat));
	return true;
}

bool JmyLog::openLib(const char* category)
{
	zlog_category_t* cat = zlog_get_category(category);
	if (!cat) return false;
	lib_str_ = category;
	lib_cate_ = cat;
	return true;
}

bool JmyLog::reload()
{
	int rc = zlog_reload(filepath_.c_str());
	if (rc != 0) {
		return false;
	}
	auto it = str2cate_.begin();
	for (; it!=str2cate_.end(); ++it) {
		zlog_category_t* cat = zlog_get_category(it->first.c_str());
		if (!cat) {
			zlog_fini();
			return false;
		}
		it->second = cat;
	}
	if (lib_str_ != "") {
		lib_cate_ = zlog_get_category(lib_str_.c_str());
	}
	return true;
}

void JmyLog::destroy()
{
	zlog_fini();
	str2cate_.clear();
	lib_str_ = "";
	lib_cate_ = NULL;
}

void JmyLog::log(const char* category,
				const char *file, size_t filelen,
				const char *func, size_t funclen,
				long line, int level,
				const char *format, ...)
{
	auto it = str2cate_.find(category);
	if (it == str2cate_.end())
		return;

	va_list vl;
	va_start(vl, format);
	vzlog(it->second, file, filelen, func, funclen, line, level, format, vl);
	va_end(vl);
}

void JmyLog::logLib(const char *file, size_t filelen,
			const char *func, size_t funclen,
			long line, int level,
			const char *format, ...)
{
	if (!lib_cate_)
		return;
	va_list vl;
	va_start(vl, format);
	vzlog(lib_cate_, file, filelen, func, funclen, line, level, format, vl);
	va_end(vl);
}
