#include "jmy_data_handler.h"
#include "jmy_tcp_session.h"
#include "jmy_tcp_connector.h"
#include <iostream>

JmyDataHandler::JmyDataHandler()
{
}

JmyDataHandler::~JmyDataHandler()
{
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

int JmyDataHandler::processData(JmySessionBuffer& recv_buffer, int session_id, void* param)
{
	char* buff = recv_buffer.getReadBuff();
	unsigned int len = recv_buffer.getReadLen();

	unsigned int nhandled = 0;
	while (true) {
		if (len-nhandled < 2) {
			recv_buffer.readLen(nhandled);
			recv_buffer.moveDataToFront();
			break;
		}

		unsigned int data_len = ((buff[nhandled]<<8)&0xff00) + (buff[nhandled+1]&0xff);
		if (len-nhandled < data_len+2) {
			// (can write_len + can read len - nhandled) is not enough to hold next message
			if (recv_buffer.getWriteLen()+len-nhandled < data_len) {
				recv_buffer.moveDataToFront();
			}
			recv_buffer.readLen(nhandled);
			break;
		}

		int msg_id = ((buff[nhandled+2]<<8)&0xff00) + (buff[nhandled+3]&0xff);
		msg_info_.msg_id = msg_id;
		msg_info_.data = buff+nhandled+2+2;
		msg_info_.len = data_len-2;
		msg_info_.session_id = session_id;
		msg_info_.param = param;
		int res = processMsg(&msg_info_);
		if (res < 0) return res;
		nhandled += (2+data_len);
		if (len - nhandled == 0) {
			recv_buffer.readLen(nhandled);
			break;
		}
	}
	return nhandled;
}

int JmyDataHandler::processData(JmySessionBuffer& recv_buffer, int session_id, std::shared_ptr<JmyTcpSessionMgr> session_mgr)
{
	return processData(recv_buffer, session_id, (void*)session_mgr.get());
}

int JmyDataHandler::processData(JmySessionBuffer& recv_buffer, JmyTcpConnector* connector)
{
	return processData(recv_buffer, 0, (void*)connector);
}

int JmyDataHandler::processData(JmyDoubleSessionBuffer* recv_buffer, int session_id, std::shared_ptr<JmyTcpSessionMgr> session_mgr)
{
	if (!recv_buffer) return -1;
	char* buff = recv_buffer->getReadBuff();
	unsigned int len = recv_buffer->getReadLen();

	unsigned int nhandled = 0;
	while (true) {
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
				std::cout << "JmyDataHandler::processData  current buffer size(" << recv_buffer->getTotalLen() << ") not enough to hold next message data(" << data_len << "), and cant malloc new large buffer" << std::endl;
				return -1;
			}
			if (data_len+2 > recv_buffer->getTotalLen()) {
				std::cout << "JmyDataHandler::processData  next message data size(" << data_len << ") is too large than max buffer size(" << recv_buffer->getTotalLen()-2 << ")" << std::endl;
				return -1;
			}
			break;
		}

		// next message length small than normal buffer size, switch to normal buffer
		if (recv_buffer->isLarge() && data_len+2 <= recv_buffer->getTotalLen()) {
			if (!recv_buffer->backToNormal()) {
				std::cout << "JmyDataHandler::processData  back to normal buffer failed" << std::endl;
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
			std::cout << "JmyDataHandler::processData  not enough length to get data. data_len: " << data_len << ", left_len: " << len-nhandled-2 << std::endl;
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
		if (len - nhandled == 0) {
			recv_buffer->readLen(nhandled);
			break;
		}
	}
	return nhandled;
}

int JmyDataHandler::writeData(JmySessionBuffer& send_buffer, int msg_id, const char* data, unsigned int len)
{
	if (!data || !len)
		return 0;

	if (!send_buffer.checkWriteLen(len+2+2)) {
		std::cout << "JmyDataHandler::writeData  data length(" << len << ") is too large to write" << std::endl;
		return -1;
	}
	return writeData<JmySessionBuffer>(&send_buffer, msg_id, data, len);
}

int JmyDataHandler::writeData(JmyDoubleSessionBuffer* send_buffer, int msg_id, const char* data, unsigned int len)
{
	if (!send_buffer || !data || !len)
		return 0;

	if (!send_buffer->checkWriteLen(len+2+2)) {
		bool too_large = true;
		if (!send_buffer->isLarge()) {
			if (!send_buffer->switchToLarge()) {
				std::cout << "JmyDataHandler::writeData  switch to large buffer failed" << std::endl;
				return -1;
			}
			if (send_buffer->checkWriteLen(len+2+2)) {
				too_large = false;
			}
		}
		if (too_large) {
			std::cout << "JmyDataHandler::writeData  data length(" << len << ") is too large to write" << std::endl;
			return -1;
		}
	}
	// check if length of data is small than normal buffer, switch to normal buffer
	if (send_buffer->isLarge() && send_buffer->getNormalLen() >= len+2+2)  {
		if (!send_buffer->backToNormal()) {
			std::cout << "JmyDataHandler::writeData  back to normal buffer failed" << std::endl;
			return -1;
		}
	}
	return writeData<JmyDoubleSessionBuffer>(send_buffer, msg_id, data, len);
}

int JmyDataHandler::processMsg(JmyMsgInfo* info)
{
	if (!info) return -1;
	std::unordered_map<int, jmy_msg_handler>::iterator it = msg_handler_map_.find(info->msg_id);
	if (it == msg_handler_map_.end()) {
		std::cout << "JmyDataHandler::processMsg  not found msg(" << info->msg_id << ") handler, session_id(" << info->session_id << ")" << std::endl;
		return info->len;
	}
	if (it->second(info) < 0)
		return -1;
	return info->len;
}
