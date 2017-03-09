#pragma once

#include <unordered_map>
#include "jmy_datatype.h"
#include "jmy_session_buffer.h"
#include "jmy_session_buffer_pool.h"
#include "jmy_tcp_session.h"
#include "jmy_log.h"

class JmyTcpConnectorMgr;

class JmyDataHandler
{
public:
	JmyDataHandler();
	~JmyDataHandler();
	bool loadMsgHandle(const JmyId2MsgHandler id2handlers[], int size);
	int processData(JmySessionBuffer& recv_buff, int session_id, void* param);
	int processData(JmySessionBuffer& recv_buff, int session_id, std::shared_ptr<JmyTcpSessionMgr> session_mgr);
	int processData(JmySessionBuffer& recv_buff, int connector_id, JmyTcpConnectorMgr* mgr);
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
		LibJmyLogError("write head failed");
		return -1;
	}
	// write msg_id
	buf[0] = (msg_id>>8)&0xff;
	buf[1] = msg_id&0xff;
	if (!send_buffer->writeData(buf, 2)) {
		LibJmyLogError("write msg_id failed");
		return -1;
	}
	// write body
	if (!send_buffer->writeData(data, len)) {
		LibJmyLogError("write data failed");
		return -1;
	}
	return len;
}
