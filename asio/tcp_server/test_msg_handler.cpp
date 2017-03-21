#include "../libjmy/jmy.h"
#include "test_msg_handler.h"
#include "util.h"
#include <chrono>

uint64_t TestMsgHandler::count_ = 0;

int TestMsgHandler::process_one(JmyMsgInfo* info)
{
	if (!info) return -1;
	const char* data = info->data;
	unsigned int len = info->len;
	int session_id = info->session_id;
#if USE_CONNECTOR_AND_SESSION
	JmyTcpSessionMgr* session_mgr = (JmyTcpSessionMgr*)info->param;
	ServerLogDebug("data(%d), len(%d)", data, len);
	JmyTcpSession* session = session_mgr->getSessionById(session_id);
	if (!session) {
		ServerLogError("session(%d) not found", session_id);
		return -1;
	}
	session->send(1, data, len);
#else
	JmyTcpConnectionMgr* conn_mgr_ = (JmyTcpConnectionMgr*)info->param;
	JmyTcpConnection* conn =  conn_mgr_->get(session_id);
	if (!conn) {
		ServerLogError("conn(%d) not found", session_id);
		return -1;
	}
	conn->send(1, data, len);
#endif
	ServerLogDebug("processed count %d", count_++);
	return len;
}
