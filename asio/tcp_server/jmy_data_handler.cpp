#include "jmy_data_handler.h"
#include "jmy_tcp_session.h"
#include <iostream>

JmyDataHandler::JmyDataHandler()
{
}

JmyDataHandler::~JmyDataHandler()
{
}

bool JmyDataHandler::loadMsgHandle(const JmyId2MsgHandler id2handlers[], int size)
{
	if (!id2handlers || size == 0)
		return false;

	int i = 0;
	for (; i<size; ++i) {
		msg_handler_map_.insert(std::make_pair(id2handlers[i].msg_id, id2handlers[i].handler));
	}

	return true;
}

int JmyDataHandler::processMsg(int msg_id, const char* data, unsigned int len, int session_id, std::shared_ptr<JmyTcpSessionMgr> session_mgr)
{
	std::unordered_map<int, jmy_msg_handler>::iterator it = msg_handler_map_.find(msg_id);
	if (it == msg_handler_map_.end()) {
		std::cout << "not found msg(" << msg_id << ") handler, session_id(" << session_id << ")" << std::endl;
		return len;
	}
	if (it->second(data, len, session_id, session_mgr) < 0)
		return -1;
	return len;
}
