#include "jmy_data_handler.h"
#include "jmy_tcp_session.h"
#include "jmy_tcp_connector.h"

JmyDataHandler::JmyDataHandler() : ack_handler_(NULL), heartbeat_handler_(NULL)
{
}

JmyDataHandler::~JmyDataHandler()
{
}

bool JmyDataHandler::registerMsgHandle(JmyId2MsgHandler id2handler)
{
	if (msg_handler_map_.find(id2handler.msg_id) != msg_handler_map_.end())
		return false;
	msg_handler_map_.insert(std::make_pair(id2handler.msg_id, id2handler.handler));
	return true;
}

bool JmyDataHandler::loadMsgHandle(const JmyId2MsgHandler id2handlers[], int size)
{
	if (!id2handlers || size == 0)
		return false;

	int i = 0;
	for (; i<size; ++i) {
		msg_handler_map_.insert(std::make_pair(id2handlers[i].msg_id, id2handlers[i].handler));
	}

	return true;
}

int JmyDataHandler::processOne(
		JmySessionBuffer& session_buffer,
		unsigned int offset,
		JmyPacketUnpackData& data,
		int session_id, void* param)
{
	const char* buff = session_buffer.getReadBuff();
	unsigned int read_len = session_buffer.getReadLen();
	int r = jmy_net_proto_unpack_data_head(buff+offset, read_len-offset, data, session_id, param);
	if (r < 0) {
		if (data.type == JmyPacketUserData && data.result == JmyPacketUnpackMsgLenInvalid) {
			LibJmyLogError("data len(%d) is invalid", data.data);
		}
		return -1;
	}

	// user data
	if (data.type == JmyPacketUserData) {
		if (data.result == JmyPacketUnpackDataNotEnough) {
			session_buffer.moveDataToFront();
			return 0;
		} else if (data.result == JmyPacketUnpackUserDataNotEnough) {
			unsigned int write_len = session_buffer.getWriteLen();
			int can_read_len = (int)(write_len + read_len - offset); 
			// (can write_len + can read len - nhandled) is not enough to hold next message
			if (can_read_len < unpack_data_.data) {
				session_buffer.moveDataToFront();
			}
			return 0;
		}

		int res = processMsg(&data.msg_info);
		if (res < 0) return res;
	}
	// ack
	else if (data.type == JmyPacketAck) {
		ack_info_.session_id = session_id;
		ack_info_.session_param = param;
		ack_info_.ack_count = (unsigned short)data.data;
		ack_info_.curr_id = (unsigned short)(int)(long)data.param;
		if (processAck(&ack_info_) < 0)
			return -1;
	}
	// heart beat
	else if (data.type == JmyPacketHeartbeat) {
		heartbeat_info_.session_id = session_id;
		if (processHeartbeat(&heartbeat_info_) < 0)
			return -1;
	}
	// other invalid packet
	else {
		return -1;
	}
	return r;
}

int JmyDataHandler::processData(JmySessionBuffer& recv_buffer, int session_id, void* param)
{
	unsigned int len = recv_buffer.getReadLen();

	unsigned int nhandled = 0;
	int count = 0;
	while (true) {
		int res = processOne(recv_buffer, nhandled, unpack_data_, session_id, param);
		if (res < 0) {
			return -1;
		}
		if (res == 0) {
			recv_buffer.readLen(nhandled);
			break;
		}
		nhandled += res;
		if (unpack_data_.type == JmyPacketUserData) {
			count += 1;
		}
		if (len - nhandled == 0) {
			recv_buffer.readLen(nhandled);
			break;
		}
	}
	return count;
}

int JmyDataHandler::processData(JmySessionBuffer& recv_buffer, int session_id, std::shared_ptr<JmyTcpSessionMgr> session_mgr)
{
	return processData(recv_buffer, session_id, (void*)session_mgr.get());
}

int JmyDataHandler::processData(JmySessionBuffer& recv_buffer, int connector_id, JmyTcpConnectorMgr* mgr)
{
	return processData(recv_buffer, connector_id, (void*)mgr);
}

int JmyDataHandler::processData(JmyDoubleSessionBuffer& recv_buffer, int session_id, std::shared_ptr<JmyTcpSessionMgr> session_mgr)
{
#if 0
		// not enough length get head
		if (len-nhandled < 2) {
			recv_buffer->readLen(nhandled);
			recv_buffer->moveDataToFront();
			break;
		}

		// data length not include head length, data_len+2 is the whole data len
		unsigned int data_len = ((buff[nhandled]<<8)&0xff00) + (buff[nhandled+1]&0xff);
		// current buffer not enough to hold next message data, switch to large buffer
		if (!recv_buffer->isLarge() && data_len+2 > recv_buffer->getTotalLen()) {
			if (!recv_buffer->switchToLarge()) {
				LibJmyLogError("current buffer size(%d) not enough to hold next message data(%d), and cant malloc new large buffer", recv_buffer->getTotalLen(), data_len);
				return -1;
			}
			if (data_len+2 > recv_buffer->getTotalLen()) {
				LibJmyLogDebug("next message data size(%d) is too large than max buffer size(%d)", data_len, recv_buffer->getTotalLen()-2);
				return -1;
			}
			break;
		}

		// next message length small than normal buffer size, switch to normal buffer
		if (recv_buffer->isLarge() && data_len+2 <= recv_buffer->getTotalLen()) {
			if (!recv_buffer->backToNormal()) {
				LibJmyLogError("back to normal buffer failed");
				return -1;
			}
		}

		// left length not whole data length
		if (len-nhandled < data_len+2) {
			// (can write_len + can read len - nhandled) is not enough to hold next message
			if (recv_buffer->getWriteLen()+len-nhandled < data_len) {
				recv_buffer->moveDataToFront();
			}
			recv_buffer->readLen(nhandled);
			break;
		}

		// msg id
		int msg_id = ((buff[nhandled+2]<<8)&0xff00) + (buff[nhandled+3]&0xff);
		msg_info_.msg_id = msg_id;
		msg_info_.data = buff+nhandled+2+2;
		msg_info_.len = data_len - 2;
		msg_info_.session_id = session_id;
		msg_info_.param = (void*)(session_mgr.get());
		int res = processMsg(&msg_info_); 
		if (res < 0) return res;
		nhandled += (data_len+2);
#endif
	JmySessionBuffer& buff = recv_buffer.getSessionBuffer();
	unsigned int len = recv_buffer.getReadLen();
	int nhandled = 0;
	int count = 0;
	while (true) {
		int res = processOne(buff, nhandled, unpack_data_, session_id, (void*)session_mgr.get());
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
		count += 1;
		if (len - nhandled == 0) {
			buff.readLen(nhandled);
			break;
		}
	}
	return count;
}

int JmyDataHandler::writeData(JmySessionBuffer& send_buffer, int msg_id, const char* data, unsigned int len)
{
	if (!data || !len)
		return 0;

	if (!send_buffer.checkWriteLen(len+UserDataHeadLen)) {
		LibJmyLogError("data length(%d) is not enough to write", len);
		return -1;
	}
	return writeData<JmySessionBuffer>(&send_buffer, msg_id, data, len);
}

int JmyDataHandler::writeData(JmyDoubleSessionBuffer* send_buffer, int msg_id, const char* data, unsigned int len)
{
	if (!send_buffer || !data || !len)
		return 0;

	if (!send_buffer->checkWriteLen(len+UserDataHeadLen)) {
		bool too_large = true;
		if (!send_buffer->isLarge()) {
			if (!send_buffer->switchToLarge()) {
				LibJmyLogError("switch to large buffer failed");
				return -1;
			}
			if (send_buffer->checkWriteLen(len+UserDataHeadLen)) {
				too_large = false;
			}
		}
		if (too_large) {
			LibJmyLogError("data length(%d) is not enough to write", len);
			return -1;
		}
	}
	// check if length of data is small than normal buffer, switch to normal buffer
	if (send_buffer->isLarge() && send_buffer->getNormalLen() >= len+UserDataHeadLen)  {
		if (!send_buffer->backToNormal()) {
			LibJmyLogError("back to normal buffer failed");
			return -1;
		}
	}
	return writeData<JmyDoubleSessionBuffer>(send_buffer, msg_id, data, len);
}

int JmyDataHandler::writeData(JmySessionBufferList* buffer_list, int msg_id, const char* data, unsigned int len)
{
	if (!buffer_list || !data || !len)
		return 0;

	return writeData<JmySessionBufferList>(buffer_list, msg_id, data, len);
}

int JmyDataHandler::processMsg(JmyMsgInfo* info)
{
	if (!info) return -1;
	std::unordered_map<int, jmy_msg_handler>::iterator it = msg_handler_map_.find(info->msg_id);
	if (it == msg_handler_map_.end()) {
		LibJmyLogWarn("not found msg(%d) handler, session_id(%d)", info->msg_id, info->session_id);
		return info->len;
	}
	if (it->second(info) < 0)
		return -1;
	return info->len;
}

int JmyDataHandler::processAck(JmyAckMsgInfo* info)
{
	if (!info) return -1;
	JmySessionInfo sinfo;
	jmy_id_to_session_info(info->session_id, sinfo);
	if (sinfo.type == SESSION_TYPE_AGENT) {
		JmyTcpSessionMgr* mgr = (JmyTcpSessionMgr*)info->session_param;
		JmyTcpSession* session = mgr->getSessionById(sinfo.session_id);
		if (!session) {
			LibJmyLogError("not found session(%d)", sinfo.session_id);
			return -1;
		}
		// get send buff
	} else if (sinfo.type == SESSION_TYPE_CONNECTOR) {
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
	return 0;
}

int processHeartbeat(JmyHeartbeatMsgInfo*)
{
	return 0;
}
