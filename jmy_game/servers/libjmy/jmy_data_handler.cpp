#include "jmy_data_handler.h"
#if USE_CONNECTOR_AND_SESSION
#include "jmy_tcp_session.h"
#include "jmy_tcp_connector.h"
#else
#include "jmy_tcp_connection.h"
#endif

JmyDataHandler::JmyDataHandler() : default_msg_handler_(nullptr)
{
}

JmyDataHandler::~JmyDataHandler()
{
}

bool JmyDataHandler::registerMsgHandle(JmyId2MsgHandler id2handler)
{
	assert(id2handler.msg_id>=JMY_MIN_MESSAGE_ID && id2handler.msg_id<=JMY_MAX_MESSAGE_ID);
	assert(id2handler.handler != nullptr);
	if (msg_handler_map_.find(id2handler.msg_id) != msg_handler_map_.end())
		return false;
	msg_handler_map_.insert(std::make_pair(id2handler.msg_id, id2handler.handler));
	return true;
}

bool JmyDataHandler::loadMsgHandle(const JmyId2MsgHandler id2handlers[], int size)
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

#if USE_CONN_PROTO
int JmyDataHandler::processConn(char* buf, unsigned char len, int session_id, void* param)
{
	int r = jmy_net_proto_unpack_data_head(buf, len, unpack_data_, session_id, param);
	if (r < 0) {
		LibJmyLogError("conn packet unpack failed!!! session_id(%d)", session_id);
		return -1;
	}

	// conn
	if (unpack_data_.type == JMY_PACKET_CONNECT) {
		conn_info_.session_id = session_id;
		conn_info_.session_param = param;
		if (handleConn(&conn_info_) < 0) {
			LibJmyLogError("handle conn failed");
			return -1;
		}
	}
	// conn res
	else if (unpack_data_.type == JMY_PACKET_CONNECT_RESULT) {
		conn_res_info_.session_id = session_id;
		conn_res_info_.session_param = param;
		conn_res_info_.info.conn_id = unpack_data_.data;
		conn_res_info_.info.session_str = (char*)unpack_data_.param;
		conn_res_info_.info.session_str_len = ConnResSessionLen;
		if (handleConnRes(&conn_res_info_) < 0) {
			LibJmyLogError("handle ack conn failed");
			return -1;
		}
	}
	// reconn
	else if (unpack_data_.type == JMY_PACKET_RECONNECT) {
		reconn_info_.session_id = session_id;
		reconn_info_.session_param = param;
		reconn_info_.info.conn_id = unpack_data_.data;
		reconn_info_.info.session_str = (char*)unpack_data_.param;
		reconn_info_.info.session_str_len = ConnResSessionLen;
		if (handleReconn(&reconn_info_) < 0) {
			LibJmyLogError("handle reconn failed");
			return -1;
		}
	}
	// reconn res
	else if (unpack_data_.type == JMY_PACKET_RECONNECT_RESULT) {
		reconn_res_info_.session_id = session_id;
		reconn_res_info_.session_param = param;
		reconn_res_info_.new_info.conn_id = unpack_data_.data;
		reconn_res_info_.new_info.session_str = (char*)unpack_data_.param;
		reconn_res_info_.new_info.session_str_len = ConnResSessionLen;
		if (handleReconnRes(&reconn_res_info_) < 0) {
			LibJmyLogError("handle ack reconn failed");
			return -1;
		}
	}
	
	return r;
}
#endif

int JmyDataHandler::processData(JmySessionBuffer& recv_buffer, int session_id, void* param)
{
	unsigned int len = recv_buffer.getReadLen();

	unsigned int nhandled = 0;
	int count = 0;
	while (true) {
		int res = handleOne(recv_buffer, nhandled, unpack_data_, session_id, param);
		if (res < 0) {
			return -1;
		}
		if (res == 0) {
			recv_buffer.readLen(nhandled);
			break;
		}
		nhandled += res;
		// must user data packet
		if (unpack_data_.type == JMY_PACKET_USER_DATA) {
			count += 1;
		}
		if (len - nhandled == 0) {
			recv_buffer.readLen(nhandled);
			break;
		}
	}
	return count;
}

#if USE_CONNECTOR_AND_SESSION
int JmyDataHandler::processData(JmySessionBuffer& recv_buffer, int session_id, std::shared_ptr<JmyTcpSessionMgr> session_mgr)
{
	return processData(recv_buffer, session_id, (void*)session_mgr.get());
}

int JmyDataHandler::processData(JmySessionBuffer& recv_buffer, int connector_id, JmyTcpConnectorMgr* mgr)
{
	return processData(recv_buffer, connector_id, (void*)mgr);
}
#endif

int JmyDataHandler::processData(JmyDoubleSessionBuffer& recv_buffer, int session_id, void* param)
{
	JmySessionBuffer& buff = recv_buffer.getSessionBuffer();
	unsigned int len = recv_buffer.getReadLen();
	int nhandled = 0;
	int count = 0;
	while (true) {
		int res = handleOne(buff, nhandled, unpack_data_, session_id, param);
		if (res < 0) {
			return -1;
		}
		// not enough data to read
		if (res == 0) {
			if (!recv_buffer.isLarge() && unpack_data_.data > (int)recv_buffer.getTotalLen()) {
				if (!recv_buffer.switchToLarge()) {
					LibJmyLogError("current buffer size(%d) not enough to hold next message data(%d), and cant malloc new large buffer", recv_buffer.getTotalLen(), unpack_data_.data);
					return -1;
				}
				if (unpack_data_.data > (int)recv_buffer.getTotalLen()) {
					LibJmyLogDebug("next message data size(%d) is too large than max buffer size(%d)", unpack_data_.data, recv_buffer.getTotalLen()-2);
					return -1;
				}
			}
			buff.readLen(nhandled);
			break;
		}
		
		// next message length small than normal buffer size, switch to normal buffer
		if (recv_buffer.isLarge() && unpack_data_.data <= (int)recv_buffer.getTotalLen()) {
			if (!recv_buffer.backToNormal()) {
				LibJmyLogError("back to normal buffer failed");
				return -1;
			}
		}
		nhandled += res;
		if (unpack_data_.type == JMY_PACKET_USER_DATA) {
			count += 1;
		}
		if (len - nhandled == 0) {
			buff.readLen(nhandled);
			break;
		}
	}
	return count;
}

int JmyDataHandler::writeUserData(JmySessionBuffer& send_buffer, int msg_id, const char* data, unsigned int len)
{
	if (!data)
		return 0;

	if (!send_buffer.checkWriteLen(len + JMY_PACKET_LEN_USER_DATA_HEAD)) {
		LibJmyLogError("data length(%d) is not enough to write", len);
		return -1;
	}
	return writeUserData<JmySessionBuffer>(&send_buffer, msg_id, data, len);
}

int JmyDataHandler::writeUserData(JmyDoubleSessionBuffer* send_buffer, int msg_id, const char* data, unsigned int len)
{
	if (!send_buffer || !data)
		return 0;

	if (!send_buffer->checkWriteLen(len + JMY_PACKET_LEN_USER_DATA_HEAD)) {
		bool too_large = true;
		if (!send_buffer->isLarge()) {
			if (!send_buffer->switchToLarge()) {
				LibJmyLogError("switch to large buffer failed");
				return -1;
			}
			if (send_buffer->checkWriteLen(len + JMY_PACKET_LEN_USER_DATA_HEAD)) {
				too_large = false;
			}
		}
		if (too_large) {
			LibJmyLogError("data length(%d) is not enough to write", len);
			return -1;
		}
	}
	// check if length of data is small than normal buffer, switch to normal buffer
	if (send_buffer->isLarge() && send_buffer->getNormalLen() >= len + JMY_PACKET_LEN_USER_DATA_HEAD)  {
		if (!send_buffer->backToNormal()) {
			LibJmyLogError("back to normal buffer failed");
			return -1;
		}
	}
	return writeUserData<JmyDoubleSessionBuffer>(send_buffer, msg_id, data, len);
}

int JmyDataHandler::writeUserData(JmySessionBufferList* buffer_list, int msg_id, const char* data, unsigned int len)
{
	if (!buffer_list || !data)
		return 0;

	return writeUserData<JmySessionBufferList>(buffer_list, msg_id, data, len);
}

int JmyDataHandler::writeUserIdAndData(JmySessionBuffer& buffer, int user_id, int msg_id, const char* data, unsigned short len)
{
	if (!data)
		return 0;

	if (!buffer.checkWriteLen(len + JMY_PACKET_LEN_USER_ID_DATA_HEAD)) {
		LibJmyLogError("data length(%d) is not enough to write", len);
		return -1;
	}
	return writeUserIdAndData<JmySessionBuffer>(&buffer, user_id, msg_id, data, len);
}

int JmyDataHandler::writeUserIdAndData(JmyDoubleSessionBuffer* buffer, int user_id, int msg_id, const char* data, unsigned short len)
{
	if (!buffer || !data)
		return 0;

	if (!buffer->checkWriteLen(len + JMY_PACKET_LEN_USER_ID_DATA_HEAD)) {
		bool too_large = true;
		if (!buffer->isLarge()) {
			if (!buffer->switchToLarge()) {
				LibJmyLogError("switch to large buffer failed");
				return -1;
			}
			if (buffer->checkWriteLen(len + JMY_PACKET_LEN_USER_ID_DATA_HEAD)) {
				too_large = false;
			}
		}
		if (too_large) {
			LibJmyLogError("data length(%d) is not enough to write", len);
			return -1;
		}
	}
	// check if length of data is small than normal buffer, switch to normal buffer
	if (buffer->isLarge() && buffer->getNormalLen() >= (unsigned int)len + JMY_PACKET_LEN_USER_ID_DATA_HEAD)  {
		if (!buffer->backToNormal()) {
			LibJmyLogError("back to normal buffer failed");
			return -1;
		}
	}
	return writeUserIdAndData<JmyDoubleSessionBuffer>(buffer, user_id, msg_id, data, len);
}

int JmyDataHandler::writeUserIdAndData(JmySessionBufferList* buffer_list, int user_id, int msg_id, const char* data, unsigned short len)
{
	if (!buffer_list || !data)
		return 0;

	return writeUserIdAndData<JmySessionBufferList>(buffer_list, user_id, msg_id, data, len);
}

int JmyDataHandler::handleMsg(JmyMsgInfo* info)
{
	if (!info) return -1;
	std::unordered_map<int, jmy_msg_handler>::iterator it = msg_handler_map_.find(info->msg_id);
	if (it == msg_handler_map_.end()) {
		if (default_msg_handler_) {
			int res = default_msg_handler_(info);
			if (res == 0) {
				LibJmyLogWarn("not found msg(%d) handler, session_id(%d)", info->msg_id, info->session_id);
				return 0;
			} else if (res < 0) {
				return -1;
			} else {
				return info->len;
			}
		} else {
			LibJmyLogWarn("not found default message handler to handle message(%d), session_id(%d)", info->msg_id, info->session_id);
			return 0;
		}
	}
	if (it->second(info) < 0)
		return -1;
	return info->len;
}

int JmyDataHandler::handleAck(JmyAckMsgInfo* info)
{
	if (!info) return -1;
#if USE_CONNECTOR_AND_SESSION
	JmySessionInfo sinfo;
	jmy_id_to_session_info(info->session_id, sinfo);
	if (sinfo.type == JMY_CONN_TYPE_PASSIVE) {
		JmyTcpSessionMgr* mgr = (JmyTcpSessionMgr*)info->session_param;
		JmyTcpSession* session = mgr->getSessionById(sinfo.session_id);
		if (!session) {
			LibJmyLogError("not found session(%d)", sinfo.session_id);
			return -1;
		}
		// get send buff
	} else if (sinfo.type == JMY_CONN_TYPE_ACTIVE) {
		JmyTcpConnectorMgr* mgr = (JmyTcpConnectorMgr*)info->session_param;
		if (!mgr) return -1;
		JmyTcpConnector* connector = mgr->get(sinfo.session_id);
		if (!connector) {
			LibJmyLogError("not found connector(%d)", sinfo.session_id);
			return -1;
		}
		// get send buff
	} else {
		LibJmyLogError("invalid session type %d", sinfo.type);
		return -1;
	}
#else
	JmyTcpConnectionMgr* mgr = (JmyTcpConnectionMgr*)info->session_param;
	JmyTcpConnection* conn = mgr->get(info->session_id);
	if (!conn) {
		LibJmyLogError("not found connection(%d)", info->session_id);
		return -1;
	}
	ack_info_.ack_info.ack_count = info->ack_info.ack_count;
	if (conn->handleAck(&ack_info_.ack_info) < 0) {
		LibJmyLogError("handle ack(ack_count:%d) failed", info->ack_info.ack_count);
		return -1;
	}
#endif
	return 0;
}

int JmyDataHandler::handleHeartbeat(JmyHeartbeatMsgInfo* info)
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

int JmyDataHandler::handleDisconnect(JmyDisconnectMsgInfo* info)
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

int JmyDataHandler::handleDisconnectAck(JmyDisconnectAckMsgInfo* info)
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

int JmyDataHandler::handleOne(
		JmySessionBuffer& session_buffer,
		unsigned int offset,
		JmyPacketUnpackData& data,
		int session_id, void* param)
{
	const char* buff = session_buffer.getReadBuff();
	unsigned int read_len = session_buffer.getReadLen();
	int r = jmy_net_proto_unpack_data_head(buff+offset, read_len-offset, data, session_id, param);
	if (r < 0) {
		if ((data.type == JMY_PACKET_USER_DATA || data.type == JMY_PACKET_USER_ID_DATA) &&
			data.result == JMY_UNPACK_RESULT_MSG_LEN_INVALID) {
			LibJmyLogError("data len(%d) is invalid", data.data);
		}
		return -1;
	} else if (r == 0 && (data.type != JMY_PACKET_USER_DATA && data.type != JMY_PACKET_USER_ID_DATA)) {
		return 0;
	}

	// user data
	if (data.type == JMY_PACKET_USER_DATA || data.type == JMY_PACKET_USER_ID_DATA) {
		if (data.result == JMY_UNPACK_RESULT_DATA_NOT_ENOUGH) {
			session_buffer.moveDataToFront();
			return 0;
		} else if (data.result == JMY_UNPACK_RESULT_USER_DATA_NOT_ENOUGH) {
			unsigned int write_len = session_buffer.getWriteLen();
			int can_read_len = (int)(write_len + read_len - offset); 
			// (can write_len + can read len - nhandled) is not enough to hold next message
			if (can_read_len < unpack_data_.data) {
				session_buffer.moveDataToFront();
			}
			return 0;
		}

		int res = handleMsg(&data.msg_info);
		if (res < 0) {
			LibJmyLogError("handle msg(%d) failed", data.msg_info.msg_id);
			return -1;
		}
	}
	// ack
	else if (data.type == JMY_PACKET_ACK) {
		ack_info_.session_id = session_id;
		ack_info_.session_param = param;
		ack_info_.ack_info.ack_count = (unsigned short)data.data;
		ack_info_.ack_info.curr_id = (unsigned short)(int)(long)data.param;
		if (handleAck(&ack_info_) < 0)
			return -1;
	}
	// heart beat
	else if (data.type == JMY_PACKET_HEARTBEAT) {
		heartbeat_info_.session_id = session_id;
		heartbeat_info_.session_param = param;
		if (handleHeartbeat(&heartbeat_info_) < 0)
			return -1;
	}
	// disconnect
	else if (data.type == JMY_PACKET_DISCONNECT) {
		disconn_info_.session_id = session_id;
		disconn_info_.session_param = param;
		if (handleDisconnect(&disconn_info_) < 0)
			return -1;
	}
	// disconnect ack
	else if (data.type == JMY_PACKET_DISCONNECT_ACK) {
		disconn_ack_info_.session_id = session_id;
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

#if USE_CONN_PROTO
// JmyTcpSession use
int JmyDataHandler::handleConn(JmyConnMsgInfo* info)
{
	// generate id and session
	unsigned int id = 0;
	char* session_str = NULL;
	if (!ConnIdSessionMgr->newIdAndSession(id, session_str)) {
		return -1;
	}
	JmySessionInfo si;
	if (!jmy_id_to_session_info(info->session_id, si)) {
		ConnIdSessionMgr->removeById(id);
		LibJmyLogError("session_id(%d) to session_info failed", info->session_id);
		return -1;
	}
	if (si.type != JMY_CONN_TYPE_PASSIVE) {
		ConnIdSessionMgr->removeById(id);
		LibJmyLogError("session type must JMY_CONN_TYPE_PASSIVE(%d), not %d", JMY_CONN_TYPE_PASSIVE, si.type);
		return -1;
	}

	JmyTcpSessionMgr* mgr = (JmyTcpSessionMgr*)info->session_param;
	JmyTcpSession* session =  mgr->getSessionById(si.session_id);
	if (!session) {
		ConnIdSessionMgr->removeById(id);
		LibJmyLogError("session(%d) is not found", si.session_id);
		return -1;
	}

	JmyConnResInfo ack_conn;
	ack_conn.conn_id = id;
	ack_conn.session_str = session_str;
	ack_conn.session_str_len = ConnResSessionLen;
	return session->sendConnRes(&ack_conn);
}

// JmyTcpConnector use
int JmyDataHandler::handleConnRes(JmyConnResMsgInfo* info)
{
	JmySessionInfo si;
	if (!jmy_id_to_session_info(info->session_id, si)) {
		LibJmyLogError("session_id(%d) to session_info failed", info->session_id);
		return -1;
	}
	if (si.type != JMY_CONN_TYPE_ACTIVE) {
		LibJmyLogError("session type must be JMY_CONN_TYPE_ACTIVE(%d), not %d", JMY_CONN_TYPE_ACTIVE, si.type);
		return -1;
	}

	JmyTcpConnectorMgr* mgr = (JmyTcpConnectorMgr*)info->session_param;
	JmyTcpConnector* connector = mgr->get(si.session_id);
	if (!connector) {
		LibJmyLogError("connector(%d) is not found", si.session_id);
		return -1;
	}
	
	connector->setConnResInfo(info->info);
	
	return 0;
}

// JmyTcpSession use
int JmyDataHandler::handleReconn(JmyReconnMsgInfo* info)
{
	JmySessionInfo si;
	if (!jmy_id_to_session_info(info->session_id, si)) {
		LibJmyLogError("session_id(%d) to session_info failed", info->session_id);
		return -1;
	}

	if (si.type != JMY_CONN_TYPE_PASSIVE) {
		LibJmyLogError("session type must be JMY_CONN_TYPE_PASSIVE(%d), not %d",
				JMY_CONN_TYPE_PASSIVE, si.type);
		return -1;
	}

	JmyTcpSessionMgr* mgr = (JmyTcpSessionMgr*)info->session_param;
	JmyTcpSession* session = mgr->getSessionById(si.session_id);
	return session->checkReconn(&info->info);
}

// JmyTcpConnector use
int JmyDataHandler::handleReconnRes(JmyReconnResMsgInfo* info)
{
	return 0;
}
#endif
