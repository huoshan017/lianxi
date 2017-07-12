#include "util.h"
#include "../libjmy/jmy_tcp_connection.h"
#include "../libjmy/jmy_log.h"
#include <random>
#include <chrono>

bool global_log_init(const char* logconfpath)
{
	if (!JmyLogInit(logconfpath)) {
		std::cout << "failed to init log file: " << logconfpath << std::endl;
		return false;
	}
	if (!JmyLogOpenLib(s_libjmy_log_cate)) {
		std::cout << "failed to create lib log category: " << s_libjmy_log_cate << std::endl;
		return false;
	}
	if (!JmyLogOpen(s_app_log_cate)) {
		std::cout << "failed to create app log category: " << s_app_log_cate << std::endl;
		return false;
	}
	return true;
}

JmyTcpConnection* get_connection(int conn_id, JmyTcpConnectionMgr* conn_mgr)
{
	if (!conn_mgr) {
		LogError("connection manager pointer is null");
		return nullptr;
	}
	JmyTcpConnection* conn = conn_mgr->get(conn_id);
	if (!conn) {
		LogError("not found connection by conn_id(%d)", conn_id);
	}
	return conn;
}

JmyTcpConnection* get_connection(JmyMsgInfo* info)
{
	JmyTcpConnectionMgr* conn_mgr = (JmyTcpConnectionMgr*)info->param;
	return get_connection(info->conn_id, conn_mgr);
}

JmyTcpConnection* get_connection(JmyEventInfo* info)
{
	JmyTcpConnectionMgr* conn_mgr = (JmyTcpConnectionMgr*)info->param;
	return get_connection(info->conn_id, conn_mgr);
}

char* get_session_code(char* session_buf, int buf_len)
{
	static char cs[] = "abcdefghijklmnopqrstuvwxyz0123456789~!@#$%^&*()_+`-={}[]:<>?,./";
	static uint32_t c = 0;
	// generate session string
	std::default_random_engine gen(std::time(0)+(c++));
	std::uniform_int_distribution<> dis(0, std::strlen(cs)-1);
	for (int i=0; i<buf_len; ++i) {
		session_buf[i] = cs[dis(gen)];
	}
	session_buf[buf_len] = '\0';
	return session_buf;
}
