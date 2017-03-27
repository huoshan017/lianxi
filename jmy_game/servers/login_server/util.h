#pragma once

#include "../common/log_util.h"

#define s_libjmy_log_cate "libjmy_log"
#define s_server_log_cate "server_log"

#define ServerLogDebug(...) \
	LogDebug(s_server_log_cate, __VA_ARGS__)

#define ServerLogInfo(...) \
	LogInfo(s_server_log_cate, __VA_ARGS__)

#define ServerLogNotice(...) \
	LogNotice(s_server_log_cate, __VA_ARGS__)

#define ServerLogWarn(...) \
	LogWarn(s_server_log_cate, __VA_ARGS__)

#define ServerLogError(...) \
	LogError(s_server_log_cate, __VA_ARGS__)

#define ServerLogFatal(...) \
	LogFatal(s_server_log_cate, __VA_ARGS__)
