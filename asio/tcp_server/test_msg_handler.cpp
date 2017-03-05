#include "test_msg_handler.h"
#include "../libjmy/jmy_tcp_session.h"
#include <chrono>
#include <iostream>

int TestMsgHandler::process_one(JmyMsgInfo* info)
{
	if (!info) return -1;
	const char* data = info->data;
	unsigned int len = info->len;
	int session_id = info->session_id;
	JmyTcpSessionMgr* session_mgr = (JmyTcpSessionMgr*)info->param;
	std::cout << "TestMsgHandler::process_one: data(" << data << "), len(" << len << ")" << std::endl;
	JmyTcpSession* session = session_mgr->getSessionById(session_id);
	if (!session) {
		std::cout << "error  TestMsgHandler::process_one: session(" << session_id << ") not found" << std::endl;
		return -1;
	}
	static int count = 0;
	count += 1;
	std::cout << "TestMsgHandler::process_one  processed count " << count << std::endl;
	session->send(1, data, len);
	return len;
}
