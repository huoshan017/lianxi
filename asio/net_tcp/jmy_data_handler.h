#pragma once

#include <unordered_map>
#include "jmy_datatype.h"
#include "jmy_session_buffer.h"
#include "jmy_session_buffer_pool.h"
#include "jmy_tcp_session.h"
#include <iostream>

class JmyTcpConnector;

class JmyDataHandler
{
public:
	JmyDataHandler();
	~JmyDataHandler();
	bool loadMsgHandle(const JmyId2MsgHandler id2handlers[], int size);
	int processData(JmySessionBuffer& recv_buff, int session_id, void* param);
	int processData(JmySessionBuffer& recv_buff, int session_id, std::shared_ptr<JmyTcpSessionMgr> session_mgr);
	int processData(JmySessionBuffer& recv_buff, JmyTcpConnector* connector);
	int processData(JmyDoubleSessionBuffer* recv_buffer, int session_id, std::shared_ptr<JmyTcpSessionMgr> session_mgr);
	int writeData(JmySessionBuffer& send_buff, int msg_id, const char* data, unsigned int len);
	int writeData(JmyDoubleSessionBuffer* write_buffer, int msg_id, const char* data, unsigned int len);

private:
	template <class SessionBuffer>
	int writeData(SessionBuffer* write_buffer, int msg_id, const char* data, unsigned int len);
	int processMsg(JmyMsgInfo*);
private:
	std::unordered_map<int, jmy_msg_handler> msg_handler_map_;
	JmyMsgInfo msg_info_;
};

template <class SessionBuffer>
int JmyDataHandler::writeData(SessionBuffer* send_buffer, int msg_id, const char* data, unsigned int len)
{
	// write head
	char buf[2];
	buf[0] = ((len+2)>>8) & 0xff;
	buf[1] = (len+2)&0xff;
	if (!send_buffer->writeData(buf, 2)) {
		std::cout << "JmyDataHandler::writeData  write head failed" << std::endl;
		return -1;
	}
	// write msg_id
	buf[0] = (msg_id>>8)&0xff;
	buf[1] = msg_id&0xff;
	if (!send_buffer->writeData(buf, 2)) {
		std::cout << "JmyDataHandler::writeData  write msg_id failed" << std::endl;
		return -1;
	}
	// write body
	if (!send_buffer->writeData(data, len)) {
		std::cout << "JmyDataHandler::writeData  write data failed" << std::endl;
		return -1;
	}
	return len;
}
