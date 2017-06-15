#pragma once

#include <unordered_map>
#include "jmy_datatype.h"
#include "jmy_session_buffer.h"
#include "jmy_net_proto2.h"
#include "jmy_log.h"

class JmyTcpConnectorMgr;

class JmyDataHandler2
{
public:
	JmyDataHandler2();
	~JmyDataHandler2();
	bool registerMsgHandle(JmyId2MsgHandler id2handler);
	bool loadMsgHandle(const JmyId2MsgHandler id2handlers[], int size);
	void setDefaultMsgHandler(jmy_msg_handler handler) { default_msg_handler_ = handler; }
	// return messages count
	int processData(JmySessionBufferList* buffer_list, int conn_id, JmyTcpConnectorMgr* mgr);
	// return write bytes count
	int writeUserData(JmySessionBufferList* buffer_list, int msg_id, const char* data, unsigned int len);
	int writeUserIdAndData(JmySessionBufferList* buffer_list, int user_id, int msg_id, const char* data, unsigned short len);

	template <class Buffer>
	int writeUserData(Buffer* buffer, int msg_id, const char* data, unsigned int len);
	template <class Buffer>
	int writeUserIdAndData(Buffer* buffer, int user_id, int msg_id, const char* data, unsigned short len);
	template <class Buffer>
	int writeHeartbeat(Buffer* buffer);
	template <class Buffer>
	int writeDisconnect(Buffer* buffer);
	template <class Buffer>
	int writeDisconnectAck(Buffer* buffer);

private:
	int handleOne(JmySessionBuffer& session_buffer, unsigned int offset, JmyPacketUnpackData& data, int session_id, void* param);
	int handleMsg(JmyMsgInfo*);
	int handleHeartbeat(JmyHeartbeatMsgInfo*);
	int handleDisconnect(JmyDisconnectMsgInfo*);
	int handleDisconnectAck(JmyDisconnectAckMsgInfo*);

private:
	std::unordered_map<int, jmy_msg_handler> msg_handler_map_;
	jmy_msg_handler default_msg_handler_;
	JmyPacketUnpackData unpack_data_;
	JmyHeartbeatMsgInfo heartbeat_info_;
	JmyDisconnectMsgInfo disconn_info_;
	JmyDisconnectAckMsgInfo disconn_ack_info_;
};

template <class SendBuffer>
int JmyDataHandler2::writeUserData(SendBuffer* buffer, int msg_id, const char* data, unsigned int len)
{
	char head_buf[jmy_net_proto2_user_data_pack_len()];
	int res = jmy_net_proto2_pack_user_data_head(head_buf, sizeof(head_buf), msg_id, len);
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
	return res+len;
}

template <class SendBuffer>
int JmyDataHandler2::writeUserIdAndData(SendBuffer* buffer, int user_id, int msg_id, const char* data, unsigned short len)
{
	char head_buf[jmy_net_proto2_user_id_data_pack_len()];
	int res = jmy_net_proto2_pack_user_id_data_head(head_buf, sizeof(head_buf), user_id, msg_id, len);
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
	return res+len;
}

template <class SendBuffer>
int JmyDataHandler2::writeHeartbeat(SendBuffer* buffer)
{
	char buf[jmy_net_proto2_heartbeat_pack_len()];
	int res = jmy_net_proto2_pack_heartbeat(buf, sizeof(buf));
	if (res < 0) {
		LibJmyLogError("pack heart beat failed");
		return -1;
	}
	if (!buffer->writeData(buf, sizeof(buf))) {
		LibJmyLogError("write heart beat failed");
		return -1;
	}
	return res;
}

template <class SendBuffer>
int JmyDataHandler2::writeDisconnect(SendBuffer* buffer)
{
	char buf[jmy_net_proto2_disconnect_pack_len()];
	int res = jmy_net_proto2_pack_disconnect(buf, sizeof(buf));
	if (res < 0) {
		LibJmyLogError("pack disconnect failed");
		return -1;
	}
	if (!buffer->writeData(buf, sizeof(buf))) {
		LibJmyLogError("write disconnect failed");
		return -1;
	}
	return res;
}

template <class SendBuffer>
int JmyDataHandler2::writeDisconnectAck(SendBuffer* buffer)
{
	char buf[jmy_net_proto2_disconnect_ack_pack_len()];
	int res = jmy_net_proto2_pack_disconnect_ack(buf, sizeof(buf));
	if (res < 0) {
		LibJmyLogError("pack disconnect ack failed");
		return -1;
	}
	if (!buffer->writeData(buf, sizeof(buf))) {
		LibJmyLogError("write disconnect ack failed");
		return -1;
	}
	return res;
}
