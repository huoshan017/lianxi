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
	bool loadMsgHandle(const JmyId2MsgHandler id2handlers[], int size);

	// conn
	int processConn(char* buf, unsigned char len, int session_id, void* param);

	// return messages count
	int processData(JmySessionBuffer& recv_buff, int session_id, void* param);
	int processData(JmySessionBuffer& recv_buff, int session_id, std::shared_ptr<JmyTcpSessionMgr> session_mgr);
	int processData(JmySessionBuffer& recv_buff, int connector_id, JmyTcpConnectorMgr* mgr);
	int processData(JmyDoubleSessionBuffer& recv_buffer, int session_id, std::shared_ptr<JmyTcpSessionMgr> session_mgr);
	// return write bytes count
	int writeData(JmySessionBuffer& buffer, int msg_id, const char* data, unsigned int len);
	int writeData(JmyDoubleSessionBuffer* buffer, int msg_id, const char* data, unsigned int len);
	int writeData(JmySessionBufferList* buffer_list, int msg_id, const char* data, unsigned int len);

	template <class SessionBuffer>
	int writeConn(SessionBuffer* buffer);
	template <class SessionBuffer>
	int writeAckConn(SessionBuffer* buffer, unsigned int id, char* session/*, unsigned char session_len*/);
	template <class SessionBuffer>
	int writeReconn(SessionBuffer* buffer, unsigned int id, char* session/*, unsigned char session_len*/);
	template <class SessionBuffer>
	int writeAckReconn(SessionBuffer* buffer, unsigned int id, char* session/*, unsigned char session_len*/);
	template <class SessionBuffer>
	int writeData(SessionBuffer* buffer, int msg_id, const char* data, unsigned int len);
	template <class SessionBuffer>
	int writeAck(SessionBuffer* buffer, unsigned short msg_count, unsigned short curr_id);
	template <class SessionBuffer>
	int writeHeartbeat(SessionBuffer* buffer);

private:
	int handleConn(JmyConnMsgInfo*);
	int handleAckConn(JmyAckConnMsgInfo*);
	int handleReconn(JmyReconnMsgInfo*);
	int handleAckReconn(JmyAckReconnMsgInfo*);
	int handleOne(JmySessionBuffer& session_buffer, unsigned int offset, JmyPacketUnpackData& data, int session_id, void* param);
	int handleMsg(JmyMsgInfo*);
	int handleAck(JmyAckMsgInfo*);
	int handleHeartbeat(JmyHeartbeatMsgInfo*);

private:
	std::unordered_map<int, jmy_msg_handler> msg_handler_map_;
	JmyPacketUnpackData unpack_data_;
	JmyAckMsgInfo ack_info_;
	JmyHeartbeatMsgInfo heartbeat_info_;
	JmyConnMsgInfo conn_info_;
	JmyAckConnMsgInfo ack_conn_info_;
	JmyReconnMsgInfo reconn_info_;
	JmyAckReconnMsgInfo ack_reconn_info_;
};

template <class SessionBuffer>
int JmyDataHandler::writeConn(SessionBuffer* buffer)
{
	char temp[PacketConnLen];
	int res = jmy_net_proto_pack_connect(temp, sizeof(temp));
	if (res < 0) {
		LibJmyLogError("pack conn failed");
		return -1;
	}
	if (!buffer->writeData(temp, res)) {
		LibJmyLogError("write conn failed");
		return -1;
	}
	return 0;
}

template <class SessionBuffer>
int JmyDataHandler::writeAckConn(SessionBuffer* buffer, unsigned int id, char* session/*, unsigned char session_len*/)
{
	char temp[PacketAckConnLen];
	int res = jmy_net_proto_pack_ack_connect(temp, sizeof(temp), id, session);
	if (res < 0) {
		LibJmyLogError("pack ack conn failed");
		return -1;
	}
	if (!buffer->writeData(temp, res)) {
		LibJmyLogError("write ack conn failed");
		return -1;
	}
	return res;
}

template <class SessionBuffer>
int JmyDataHandler::writeReconn(SessionBuffer* buffer, unsigned int id, char* session/*, unsigned char session_len*/)
{
	char temp[PacketReconnLen];
	int res = jmy_net_proto_pack_reconnect(temp, sizeof(temp), id, session);
	if (res < 0) {
		LibJmyLogError("pack reconn failed");
		return -1;
	}
	if (!buffer->writeData(temp, res)) {
		LibJmyLogError("write reconn failed");
		return -1;
	}
	return res;
}

template <class SessionBuffer>
int JmyDataHandler::writeAckReconn(SessionBuffer* buffer, unsigned int id, char* session/*, unsigned char session_len*/)
{
	char temp[PacketAckReconnLen];
	int res = jmy_net_proto_pack_ack_reconnect(temp, sizeof(temp), id, session);
	if (res < 0) {
		LibJmyLogError("pack ack reconn failed");
		return -1;
	}
	if (!buffer->writeData(temp, res)) {
		LibJmyLogError("pack ack reconn failed");
		return -1;
	}
	return res;
}

template <class SessionBuffer>
int JmyDataHandler::writeData(SessionBuffer* buffer, int msg_id, const char* data, unsigned int len)
{
	char head_buf[16];
	int res = jmy_net_proto_pack_msgid(head_buf, sizeof(head_buf), msg_id, len);
	if (res < 0) {
		LibJmyLogError("pack msgid(%d) failed", msg_id);
		return -1;
	}

	JmyData datas[2];
	datas[0].data = head_buf;
	datas[0].len = res;
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
int JmyDataHandler::writeAck(SessionBuffer* buffer, unsigned short ack_count, unsigned short curr_id)
{
	char buf[8];
	int res = jmy_net_proto_pack_ack(buf, sizeof(buf), ack_count, curr_id);
	if (res < 0) {
		LibJmyLogError("pack ack(msg_count:%d) failed", ack_count);
		return -1;
	}
	if (!buffer->writeData(buf, res)) {
		LibJmyLogError("write ack failed!!! msg_count(%d)", ack_count);
		return -1;
	}
	return 0;
}

template <class SessionBuffer>
int JmyDataHandler::writeHeartbeat(SessionBuffer* buffer)
{
	char buf[8];
	int res = jmy_net_proto_pack_heartbeat(buf, sizeof(buf));
	if (res < 0) {
		LibJmyLogError("pack heart beat failed");
		return -1;
	}
	if (!buffer->writeData(buf, sizeof(buf))) {
		LibJmyLogWarn("write heart beat failed");
		return -1;
	}
	return 0;
}
