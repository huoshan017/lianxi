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

char* get_session_code(char* session_buf, int buf_len)
{
	static char cs[] = "abcdefghijklmnopqrstuvwxyz0123456789~!@#$%^&*()_+`-={}[]:<>?,./";
	// generate session string
	std::default_random_engine gen;
	std::uniform_int_distribution<> dis(0, sizeof(cs));
	for (int i=0; i<buf_len; ++i) {
		session_buf[i] = dis(gen);
	}
	return session_buf;
}
