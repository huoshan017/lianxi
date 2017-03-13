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
	bool b = false;
	for (unsigned long i=0; i<sizeof(cates_)/sizeof(cates_[0]); ++i) {
		if (cates_[i].cate_str == "") {
			cates_[i].cate_str = category;
			cates_[i].cate = cat;
			b = true;
			break;
		}
	}
	return b;
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
	for (unsigned long i=0; i<sizeof(cates_)/sizeof(cates_[0]); ++i) {
		zlog_category_t* cat = zlog_get_category(cates_[i].cate_str.c_str());
		if (!cat) {
			zlog_fini();
			return false;
		}
	}
	if (lib_str_ != "") {
		lib_cate_ = zlog_get_category(lib_str_.c_str());
	}
	return true;
}

void JmyLog::destroy()
{
	zlog_fini();
	for (unsigned long i=0; i<sizeof(cates_)/sizeof(cates_[i]); ++i) {
		if (cates_[i].cate_str != "") {
			cates_[i].cate_str = "";
		}
	}
	lib_str_ = "";
	lib_cate_ = NULL;
}

void JmyLog::log(const char* category,
				const char *file, size_t filelen,
				const char *func, size_t funclen,
				long line, int level,
				const char *format, ...)
{
	zlog_category_t* c = NULL;
	unsigned long i = 0;
	unsigned long s = sizeof(cates_)/sizeof(cates_[0]);
	for (; i<s; ++i) {
		if (cates_[i].cate_str == category) {
			c = cates_[i].cate;
			break;
		}
	}
	if (!c) return;
	va_list vl;
	va_start(vl, format);
	vzlog(c, file, filelen, func, funclen, line, level, format, vl);
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
