#pragma once

#include "../libjmy/jmy_util.h"
#include "defines.h"
#include <ctime>

#define s_libjmy_log_cate "libjmy_log"
#define s_app_log_cate "app_log"

#define LogDebug(...) \
	JmyLogDebug(s_app_log_cate, __VA_ARGS__)

#define LogInfo(...) \
	JmyLogInfo(s_app_log_cate, __VA_ARGS__)

#define LogNotice(...) \
	JmyLogNotice(s_app_log_cate, __VA_ARGS__)

#define LogWarn(...) \
	JmyLogWarn(s_app_log_cate, __VA_ARGS__)

#define LogError(...) \
	JmyLogError(s_app_log_cate, __VA_ARGS__)

#define LogFatal(...) \
	JmyLogFatal(s_app_log_cate, __VA_ARGS__)

bool global_log_init(const char* logconfpath);

class JmyTcpConnection;
class JmyTcpConnectionMgr;
JmyTcpConnection* get_connection(int conn_id, JmyTcpConnectionMgr* conn_mgr);

struct JmyMsgInfo;
JmyTcpConnection* get_connection(JmyMsgInfo*);

struct JmyEventInfo;
JmyTcpConnection* get_connection(JmyEventInfo*);

char* get_session_code(char*, int);

inline int get_server_type(int server_id) {
	return server_id/SERVER_ID_DEVIDE;
}
