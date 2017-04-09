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
	void setDefaultMsgHandler(jmy_msg_handler handler) { default_msg_handler_ = handler; }

#if USE_CONN_PROTO
	// conn
	int processConn(char* buf, unsigned char len, int session_id, void* param);
#endif

	// return messages count
	int processData(JmySessionBuffer& recv_buff, int session_id, void* param);
#if USE_CONNECTOR_AND_SESSION
	int processData(JmySessionBuffer& recv_buff, int session_id, std::shared_ptr<JmyTcpSessionMgr> session_mgr);
	int processData(JmySessionBuffer& recv_buff, int connector_id, JmyTcpConnectorMgr* mgr);
#endif
	int processData(JmyDoubleSessionBuffer& recv_buffer, int session_id, void* param);
	// return write bytes count
	int writeUserData(JmySessionBuffer& buffer, int msg_id, const char* data, unsigned int len);
	int writeUserData(JmyDoubleSessionBuffer* buffer, int msg_id, const char* data, unsigned int len);
	int writeUserData(JmySessionBufferList* buffer_list, int msg_id, const char* data, unsigned int len);
	int writeUserIdAndData(JmySessionBuffer& buffer, int user_id, int msg_id, const char* data, unsigned short len);
	int writeUserIdAndData(JmyDoubleSessionBuffer* buffer, int user_id, int msg_id, const char* data, unsigned short len);
	int writeUserIdAndData(JmySessionBufferList* buffer_list, int user_id, int msg_id, const char* data, unsigned short len);

#if USE_CONN_PROTO
	template <class Buffer>
	int writeConn(Buffer* buffer);
	template <class Buffer>
	int writeConnRes(Buffer* buffer, unsigned int id, char* session/*, unsigned char session_len*/);
	template <class Buffer>
	int writeReconn(Buffer* buffer, unsigned int id, char* session/*, unsigned char session_len*/);
	template <class Buffer>
	int writeReconnRes(Buffer* buffer, unsigned int id, char* session/*, unsigned char session_len*/);
#endif
	template <class Buffer>
	int writeUserData(Buffer* buffer, int msg_id, const char* data, unsigned int len);
	template <class Buffer>
	int writeUserIdAndData(Buffer* buffer, int user_id, int msg_id, const char* data, unsigned short len);
	template <class Buffer>
	int writeAck(Buffer* buffer, unsigned short msg_count, unsigned short curr_id);
	template <class Buffer>
	int writeHeartbeat(Buffer* buffer);
	template <class Buffer>
	int writeDisconnect(Buffer* buffer);
	template <class Buffer>
	int writeDisconnectAck(Buffer* buffer);

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
	int handleDisconnect(JmyDisconnectMsgInfo*);
	int handleDisconnectAck(JmyDisconnectAckMsgInfo*);

private:
	std::unordered_map<int, jmy_msg_handler> msg_handler_map_;
	jmy_msg_handler default_msg_handler_;
	JmyPacketUnpackData unpack_data_;
	JmyAckMsgInfo ack_info_;
	JmyHeartbeatMsgInfo heartbeat_info_;
	JmyDisconnectMsgInfo disconn_info_;
	JmyDisconnectAckMsgInfo disconn_ack_info_;
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
	char temp[JMY_PACKET_LEN_CONN];
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
	char temp[JMY_PACKET_LEN_CONN_RES];
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
	char temp[JMY_PACKET_LEN_RECONN];
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
	char temp[JMY_PACKET_LEN_RECONN_RES];
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
int JmyDataHandler::writeUserData(SessionBuffer* buffer, int msg_id, const char* data, unsigned int len)
{
	char head_buf[JMY_PACKET_LEN_USER_DATA_HEAD];
	int res = jmy_net_proto_pack_msgid_datalen(head_buf, sizeof(head_buf), msg_id, len);
	if (res < 0) {
		LibJmyLogError("pack msgid(%d) failed", msg_id);
		return -1;
	}

	if (len == 0) {
		if (!buffer->writeData(head_buf, res)) {
			LibJmyLogError("write length 0 data failed");
			return -1;
		}
		return len;
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

template <class Buffer>
int JmyDataHandler::writeUserIdAndData(Buffer* buffer, int user_id, int msg_id, const char* data, unsigned short len)
{
	char head_buf[JMY_PACKET_LEN_USER_ID_DATA_HEAD];
	int res = jmy_net_proto_pack_userid_msgid_datalen(head_buf, sizeof(head_buf), user_id, msg_id, len);
	if (res < 0) {
		LibJmyLogError("pack userid(%d) msgid(%d) datelen(%d) failed", user_id, msg_id, len);
	}
	if (len == 0) {
		if (!buffer->writeData(head_buf, res)) {
			LibJmyLogError("write length 0 data failed");
			return -1;
		}
		return len;
	}

	JmyData datas[2];
	datas[0].data = head_buf;
	datas[0].len = res;
	datas[1].data = data;
	datas[1].len = len;
	if (!buffer->writeData(datas, sizeof(datas)/sizeof(datas[0]))) {
		LibJmyLogError("write data failed");
		return -1;
	}
	return len;
}

template <class SessionBuffer>
int JmyDataHandler::writeAck(SessionBuffer* buffer, unsigned short ack_count, unsigned short curr_id)
{
	char buf[JMY_PACKET_LEN_ACK];
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
	char buf[JMY_PACKET_LEN_HEARTBEAT];
	int res = jmy_net_proto_pack_heartbeat(buf, sizeof(buf));
	if (res < 0) {
		LibJmyLogError("pack heart beat failed");
		return -1;
	}
	if (!buffer->writeData(buf, sizeof(buf))) {
		LibJmyLogError("write heart beat failed");
		return -1;
	}
	return 0;
}

template <class SessionBuffer>
int JmyDataHandler::writeDisconnect(SessionBuffer* buffer)
{
	char buf[JMY_PACKET_LEN_DISCONNECT];
	int res = jmy_net_proto_pack_disconnect(buf, sizeof(buf));
	if (res < 0) {
		LibJmyLogError("pack disconnect failed");
		return -1;
	}
	if (!buffer->writeData(buf, sizeof(buf))) {
		LibJmyLogError("write disconnect failed");
		return -1;
	}
	return 0;
}

template <class SessionBuffer>
int JmyDataHandler::writeDisconnectAck(SessionBuffer* buffer)
{
	char buf[JMY_PACKET_LEN_DISCONNECT_ACK];
	int res = jmy_net_proto_pack_disconnect_ack(buf, sizeof(buf));
	if (res < 0) {
		LibJmyLogError("pack disconnect ack failed");
		return -1;
	}
	if (!buffer->writeData(buf, sizeof(buf))) {
		LibJmyLogError("write disconnect ack failed");
		return -1;
	}
	return 0;
}
