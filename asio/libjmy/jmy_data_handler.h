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
	bool registerMsgHandle(JmyId2MsgHandler id2handler);
	bool loadMsgHandle(const JmyId2MsgHandler id2handlers[], int size);
	int processData(JmySessionBuffer& recv_buff, int session_id, void* param);
	int processData(JmySessionBuffer& recv_buff, int session_id, std::shared_ptr<JmyTcpSessionMgr> session_mgr);
	int processData(JmySessionBuffer& recv_buff, int connector_id, JmyTcpConnectorMgr* mgr);
	int processData(JmyDoubleSessionBuffer* recv_buffer, int session_id, std::shared_ptr<JmyTcpSessionMgr> session_mgr);
	int writeData(JmySessionBuffer& send_buff, int msg_id, const char* data, unsigned int len);
	int writeData(JmyDoubleSessionBuffer* write_buffer, int msg_id, const char* data, unsigned int len);
	int writeData(JmySessionBufferList* buffer_list, int msg_id, const char* data, unsigned int len);

private:
	template <class SessionBuffer>
	int writeData(SessionBuffer* write_buffer, int msg_id, const char* data, unsigned int len);
	int processMsg(JmyMsgInfo*);
private:
	std::unordered_map<int, jmy_msg_handler> msg_handler_map_;
	JmyMsgInfo msg_info_;
};

template <class SessionBuffer>
int JmyDataHandler::writeData(SessionBuffer* buffer, int msg_id, const char* data, unsigned int len)
{
	JmyData datas[2];

	// len
	char head_buf[4];
	head_buf[0] = ((len+2)>>8) & 0xff;
	head_buf[1] = (len+2) & 0xff;	

	// msg_id
	head_buf[2] = (msg_id>>8) & 0xff;
	head_buf[3] = msg_id & 0xff;

	datas[0].data = head_buf;
	datas[0].len = 4;

	datas[1].data = data;
	datas[1].len = len;

	// write
	if (!buffer->writeData(datas, sizeof(datas)/sizeof(datas[0]))) {
		LibJmyLogError("write data failed");
		return -1;
	}
	return len;
}
