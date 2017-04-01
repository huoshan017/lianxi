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

bool global_log_init(const char* logconfpath);

class JmyTcpConnection;
class JmyTcpConnectionMgr;
JmyTcpConnection* get_connection(int conn_id, JmyTcpConnectionMgr* conn_mgr);

struct JmyMsgInfo;
JmyTcpConnection* get_connection(JmyMsgInfo*);

struct JmyEventInfo;
JmyTcpConnection* get_connection(JmyEventInfo*);

char* get_session_code(char*, int);
