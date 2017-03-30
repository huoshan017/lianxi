#pragma once

#include "../libjmy/jmy_util.h"

#define s_libjmy_log_cate "libjmy_log"
#define s_server_log_cate "server_log"

#define ServerLogDebug(...) \
	JmyLogDebug(s_server_log_cate, __VA_ARGS__)

#define ServerLogInfo(...) \
	JmyLogInfo(s_server_log_cate, __VA_ARGS__)

#define ServerLogNotice(...) \
	JmyLogNotice(s_server_log_cate, __VA_ARGS__)

#define ServerLogWarn(...) \
	JmyLogWarn(s_server_log_cate, __VA_ARGS__)

#define ServerLogError(...) \
	JmyLogError(s_server_log_cate, __VA_ARGS__)

#define ServerLogFatal(...) \
	JmyLogFatal(s_server_log_cate, __VA_ARGS__)


struct JmyMsgInfo;
class JmyTcpConnection;
JmyTcpConnection* get_connection(JmyMsgInfo*);

char* get_session_code(char*, int);
