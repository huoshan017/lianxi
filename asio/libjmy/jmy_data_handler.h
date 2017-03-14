#pragma once

#include <unordered_map>
#include "jmy_datatype.h"
#include "jmy_session_buffer.h"
#include "jmy_session_buffer_pool.h"
#include "jmy_tcp_session.h"
#include "jmy_net_proto.h"
#include "jmy_log.h"

class JmyTcpConnectorMgr;

class JmyDataHandler
{
public:
	JmyDataHandler();
	~JmyDataHandler();
	bool registerMsgHandle(JmyId2MsgHandler id2handler);
	bool registerAckHandle(jmy_ack_handler handler);
	bool registerHeartbeatHandle(jmy_heartbeat_handler handler);
	bool loadMsgHandle(const JmyId2MsgHandler id2handlers[], int size);
	int processData(JmySessionBuffer& recv_buff, int session_id, void* param);
	int processData(JmySessionBuffer& recv_buff, int session_id, std::shared_ptr<JmyTcpSessionMgr> session_mgr);
	int processData(JmySessionBuffer& recv_buff, int connector_id, JmyTcpConnectorMgr* mgr);
	int processData(JmyDoubleSessionBuffer& recv_buffer, int session_id, std::shared_ptr<JmyTcpSessionMgr> session_mgr);
	int writeData(JmySessionBuffer& buffer, int msg_id, const char* data, unsigned int len);
	int writeData(JmyDoubleSessionBuffer* buffer, int msg_id, const char* data, unsigned int len);
	int writeData(JmySessionBufferList* buffer_list, int msg_id, const char* data, unsigned int len);

private:
	template <class SessionBuffer>
	int writeData(SessionBuffer* buffer, int msg_id, const char* data, unsigned int len);
	template <class SessionBuffer>
	int writeAck(SessionBuffer* buffer, unsigned short msg_count);
	template <class SessionBuffer>
	int writeHeartbeat(SessionBuffer* buffer);

	int processOne(JmySessionBuffer& session_buffer, unsigned int offset, JmyPacketUnpackData& data, int session_id, void* param);
	int processMsg(JmyMsgInfo*);

private:
	std::unordered_map<int, jmy_msg_handler> msg_handler_map_;
	JmyPacketUnpackData unpack_data_;
	jmy_ack_handler ack_handler_;
	jmy_heartbeat_handler heartbeat_handler_;
};

template <class SessionBuffer>
int JmyDataHandler::writeData(SessionBuffer* buffer, int msg_id, const char* data, unsigned int len)
{
	char head_buf[16];
	if (!jmy_net_proto_pack_msgid(head_buf, sizeof(head_buf), msg_id)) {
		LibJmyLogError("pack msgid(%d) failed", msg_id);
		return -1;
	}

	JmyData datas[2];
	datas[0].data = head_buf;
	datas[0].len = sizeof(head_buf);
	datas[1].data = data;
	datas[1].len = len;

	// write
	if (!buffer->writeData(datas, sizeof(datas)/sizeof(datas[0]))) {
		LibJmyLogError("write data failed");
		return -1;
	}
	return len;
}

template <class SessionBuffer>
int JmyDataHandler::writeAck(SessionBuffer* buffer, unsigned short msg_count)
{
	char buf[8];
	if (!jmy_net_proto_pack_ack(buf, sizeof(buf), msg_count)) {
		LibJmyLogError("pack ack(msg_count:%d) failed", msg_count);
		return -1;
	}
	if (!buffer->writeData(buf, sizeof(buf))) {
		LibJmyLogError("write ack failed!!! msg_count(%d)", msg_count);
		return -1;
	}
	return 0;
}

template <class SessionBuffer>
int JmyDataHandler::writeHeartbeat(SessionBuffer* buffer)
{
	char buf[8];
	if (!jmy_net_proto_pack_heartbeat(buf, sizeof(buf))) {
		LibJmyLogError("pack heart beat failed");
		return -1;
	}
	if (!buffer->writeData(buf, sizeof(buf))) {
		LibJmyLogWarn("write heart beat failed");
		return -1;
	}
	return 0;
}
