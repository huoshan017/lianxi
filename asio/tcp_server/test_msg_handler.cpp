#include "test_msg_handler.h"
#include "jmy_tcp_session.h"
#include <iostream>

int TestMsgHandler::process_one(const char* data, unsigned int len, int session_id, std::shared_ptr<JmyTcpSessionMgr> session_mgr)
{
	std::cout << "TestMsgHandler::process_one: data(" << data << "), len(" << len << ")" << std::endl;
	JmyTcpSession* session = session_mgr->getSessionById(session_id);
	if (!session) {
		std::cout << "error  TestMsgHandler::process_one: session(" << session_id << ") not found" << std::endl;
		return -1;
	}
	session->send(data, len);
	return len;
}
