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
	ServerLogDebug("TestMsgHandler::process_one: data(%d), len(%d)", data, len);
	JmyTcpSession* session = session_mgr->getSessionById(session_id);
	if (!session) {
		ServerLogError("error  TestMsgHandler::process_one: session(%d) not found", session_id);
		return -1;
	}
	session->send(1, data, len);
#else
#endif
	ServerLogDebug("TestMsgHandler::process_one  processed count ", count_++);
	return len;
}
