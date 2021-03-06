#include "jmy_data_handler2.h"
#include "jmy_tcp_connection.h"
#include <assert.h>

JmyDataHandler2::JmyDataHandler2() : default_msg_handler_(nullptr)
{
}

JmyDataHandler2::~JmyDataHandler2()
{
}

bool JmyDataHandler2::registerMsgHandle(JmyId2MsgHandler id2handler)
{
	assert(id2handler.msg_id>=JMY_MIN_MESSAGE_ID && id2handler.msg_id<=JMY_MAX_MESSAGE_ID);
	assert(id2handler.handler != nullptr);
	if (msg_handler_map_.find(id2handler.msg_id) != msg_handler_map_.end())
		return false;
	msg_handler_map_.insert(std::make_pair(id2handler.msg_id, id2handler.handler));
	return true;
}

bool JmyDataHandler2::loadMsgHandle(const JmyId2MsgHandler id2handlers[], int size)
{
	if (!id2handlers || size == 0) {
		LibJmyLogError("id2handlers(0x%x), size(%d)", id2handlers, size);
		return false;
	}

	int i = 0;
	for (; i<size; ++i) {
		msg_handler_map_.insert(std::make_pair(id2handlers[i].msg_id, id2handlers[i].handler));
	}

	return true;
}

int JmyDataHandler2::processData(JmySessionBufferList* buffer_list, int conn_id, JmyTcpConnectionMgr* mgr)
{
	std::list<JmySessionBufferList::buffer>& buff_list = buffer_list->getBufferList();
	int nhandled = 0;
	int count = 0;
	std::list<JmySessionBufferList::buffer>::iterator it = buff_list.begin();
	for (; it!=buff_list.end(); ++it) {
		JmySessionBufferList::buffer& buff = *it;
		int res = handleOne(buff, unpack_data_, conn_id, mgr);
		if (res < 0) {
			return -1;
		}
		// not enough data to read
		nhandled += res;
		if (unpack_data_.type == JMY_PACKET2_USER_DATA || unpack_data_.type == JMY_PACKET2_USER_ID_DATA) {
			count += 1;
		}
	}
	buffer_list->clear();
	return count;
}

int JmyDataHandler2::writeUserData(JmySessionBufferList* buffer_list, int msg_id, const char* data, unsigned int len)
{
	if (!buffer_list || !data)
		return 0;

	return writeUserData<JmySessionBufferList>(buffer_list, msg_id, data, len);
}

int JmyDataHandler2::writeUserIdAndData(JmySessionBufferList* buffer_list, int user_id, int msg_id, const char* data, unsigned short len)
{
	if (!buffer_list || !data)
		return 0;

	return writeUserIdAndData<JmySessionBufferList>(buffer_list, user_id, msg_id, data, len);
}

int JmyDataHandler2::handleMsg(JmyMsgInfo* info)
{
	if (!info) return -1;
	std::unordered_map<int, jmy_msg_handler>::iterator it = msg_handler_map_.find(info->msg_id);
	if (it == msg_handler_map_.end()) {
		if (default_msg_handler_) {
			int res = default_msg_handler_(info);
			if (res == 0) {
				//LibJmyLogWarn("not found msg(%d) handler, conn_id(%d)", info->msg_id, info->conn_id);
				return 0;
			} else if (res < 0) {
				return -1;
			} else {
				return info->len;
			}
		} else {
			LibJmyLogWarn("not found default message handler to handle message(%d), conn_id(%d)", info->msg_id, info->conn_id);
			return 0;
		}
	}

	if (it->second(info) < 0)
		return -1;

	return info->len;
}

int JmyDataHandler2::handleHeartbeat(JmyHeartbeatMsgInfo* info)
{
	if (!info) return -1;
	JmyTcpConnectionMgr* mgr = (JmyTcpConnectionMgr*)info->session_param;
	JmyTcpConnection* conn = mgr->get(info->session_id);
	if (!conn) {
		LibJmyLogError("not found connection(%d)", info->session_id);
		return -1;
	}
	return conn->handleHeartbeat();
}

int JmyDataHandler2::handleDisconnect(JmyDisconnectMsgInfo* info)
{
	if (!info) return -1;
	JmyTcpConnectionMgr* mgr = (JmyTcpConnectionMgr*)info->session_param;
	JmyTcpConnection* conn = mgr->get(info->session_id);
	if (!conn) {
		LibJmyLogError("not found connection(%d)", info->session_id);
		return -1;
	}
	return conn->handleDisconnect();
}

int JmyDataHandler2::handleDisconnectAck(JmyDisconnectAckMsgInfo* info)
{
	if (!info) return -1;
	JmyTcpConnectionMgr* mgr = (JmyTcpConnectionMgr*)info->session_param;
	JmyTcpConnection* conn = mgr->get(info->session_id);
	if (!conn) {
		LibJmyLogError("not found connection(%d)", info->session_id);
		return -1;
	}
	return conn->handleDisconnectAck();
}

int JmyDataHandler2::handleOne(
		JmySessionBufferList::buffer& buff, 
		JmyPacketUnpackData2& data,
		int conn_id, void* param)
{
	JmyPacketType2 type = (JmyPacketType2)buff.woffset_;
	int r = jmy_net_proto2_unpack_data(type, buff.data_, buff.len_, data, conn_id, param);
	if (r < 0) {
		if ((type == JMY_PACKET2_USER_DATA || type == JMY_PACKET2_USER_ID_DATA) &&
			data.result == JMY_UNPACK2_RESULT_MSG_LEN_INVALID) {
			LibJmyLogError("data len(%d) is invalid", data.data);
		}
		return -1;
	} else if (r == 0 && (type != JMY_PACKET2_USER_DATA && type != JMY_PACKET2_USER_ID_DATA)) {
		return 0;
	}

	// user data
	if (type == JMY_PACKET2_USER_DATA || type == JMY_PACKET2_USER_ID_DATA) {
		int res = handleMsg(&data.msg_info);
		if (res < 0) {
			LibJmyLogError("handle msg(%d) failed", data.msg_info.msg_id);
			return -1;
		}
	}
	// heart beat
	else if (type == JMY_PACKET2_HEARTBEAT) {
		heartbeat_info_.session_id = conn_id;
		heartbeat_info_.session_param = param;
		if (handleHeartbeat(&heartbeat_info_) < 0)
			return -1;
	}
	// disconnect
	else if (type == JMY_PACKET2_DISCONNECT) {
		disconn_info_.session_id = conn_id;
		disconn_info_.session_param = param;
		if (handleDisconnect(&disconn_info_) < 0)
			return -1;
	}
	// disconnect ack
	else if (type == JMY_PACKET2_DISCONNECT_ACK) {
		disconn_ack_info_.session_id = conn_id;
		disconn_ack_info_.session_param = param;
		if (handleDisconnectAck(&disconn_ack_info_) < 0)
			return -1;
	}
	// other invalid packet
	else {
		return -1;
	}
	return r;
}
