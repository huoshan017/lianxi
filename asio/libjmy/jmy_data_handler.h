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

#if USE_CONN_PROTO
	// conn
	int processConn(char* buf, unsigned char len, int session_id, void* param);
#endif

	// return messages count
	int processData(JmySessionBuffer& recv_buff, int session_id, void* param);
	int processData(JmySessionBuffer& recv_buff, int session_id, std::shared_ptr<JmyTcpSessionMgr> session_mgr);
	int processData(JmySessionBuffer& recv_buff, int connector_id, JmyTcpConnectorMgr* mgr);
	int processData(JmyDoubleSessionBuffer& recv_buffer, int session_id, void* param);
	// return write bytes count
	int writeData(JmySessionBuffer& buffer, int msg_id, const char* data, unsigned int len);
	int writeData(JmyDoubleSessionBuffer* buffer, int msg_id, const char* data, unsigned int len);
	int writeData(JmySessionBufferList* buffer_list, int msg_id, const char* data, unsigned int len);

#if USE_CONN_PROTO
	template <class SessionBuffer>
	int writeConn(SessionBuffer* buffer);
	template <class SessionBuffer>
	int writeConnRes(SessionBuffer* buffer, unsigned int id, char* session/*, unsigned char session_len*/);
	template <class SessionBuffer>
	int writeReconn(SessionBuffer* buffer, unsigned int id, char* session/*, unsigned char session_len*/);
	template <class SessionBuffer>
	int writeReconnRes(SessionBuffer* buffer, unsigned int id, char* session/*, unsigned char session_len*/);
#endif
	template <class SessionBuffer>
	int writeData(SessionBuffer* buffer, int msg_id, const char* data, unsigned int len);
	template <class SessionBuffer>
	int writeAck(SessionBuffer* buffer, unsigned short msg_count, unsigned short curr_id);
	template <class SessionBuffer>
	int writeHeartbeat(SessionBuffer* buffer);

private:
#if USE_CONN_PROTO
	int handleConn(JmyConnMsgInfo*);
	int handleConnRes(JmyConnResMsgInfo*);
	int handleReconn(JmyReconnMsgInfo*);
	int handleReconnRes(JmyReconnResMsgInfo*);
#endif

	int handleOne(JmySessionBuffer& session_buffer, unsigned int offset, JmyPacketUnpackData& data, int session_id, void* param);
	int handleMsg(JmyMsgInfo*);
	int handleAck(JmyAckMsgInfo*);
	int handleHeartbeat(JmyHeartbeatMsgInfo*);

private:
	std::unordered_map<int, jmy_msg_handler> msg_handler_map_;
	JmyPacketUnpackData unpack_data_;
	JmyAckMsgInfo ack_info_;
	JmyHeartbeatMsgInfo heartbeat_info_;
#if USE_CONN_PROTO
	JmyConnMsgInfo conn_info_;
	JmyConnResMsgInfo conn_res_info_;
	JmyReconnMsgInfo reconn_info_;
	JmyReconnResMsgInfo reconn_res_info_;
#endif
};

#if USE_CONN_PROTO
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
int JmyDataHandler::writeConnRes(SessionBuffer* buffer, unsigned int id, char* session/*, unsigned char session_len*/)
{
	char temp[PacketConnResLen];
	int res = jmy_net_proto_pack_connect_result(temp, sizeof(temp), id, session);
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
int JmyDataHandler::writeReconnRes(SessionBuffer* buffer, unsigned int id, char* session/*, unsigned char session_len*/)
{
	char temp[PacketReconnResLen];
	int res = jmy_net_proto_pack_reconnect_result(temp, sizeof(temp), id, session);
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
#endif

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
