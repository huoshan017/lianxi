#include "util.h"
#include "../libjmy/jmy_tcp_connection.h"

JmyTcpConnection* get_connection(JmyMsgInfo* info)
{
	JmyTcpConnectionMgr* conn_mgr = (JmyTcpConnectionMgr*)info->param;
	if (!conn_mgr) {
		ServerLogError("connection manager pointer is null");
		return nullptr;
	}
	JmyTcpConnection* conn = conn_mgr->get(info->session_id);
	if (!conn) {
		ServerLogError("not found connection by session_id(%d)", info->session_id);
		return nullptr;
	}
	return conn;
}
