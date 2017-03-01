#pragma once

#include <unordered_map>
#include "jmy_datatype.h"
#include "jmy_session_buffer.h"
#include "jmy_session_buffer_pool.h"
#include "jmy_tcp_session.h"
#include <iostream>

class JmyTcpConnector;

class JmyDataHandler
{
public:
	JmyDataHandler();
	~JmyDataHandler();
	bool loadMsgHandle(const JmyId2MsgHandler id2handlers[], int size);
	int processData(JmySessionBuffer& recv_buff, int session_id, void* param);
	int processData(JmySessionBuffer& recv_buff, int session_id, std::shared_ptr<JmyTcpSessionMgr> session_mgr);
	int processData(JmySessionBuffer& recv_buff, JmyTcpConnector* connector);
	template <class SessionBuffer>
	int processData(SessionBuffer* recv_buffer, int session_id, std::shared_ptr<JmyTcpSessionMgr> session_mgr);
	int writeData(JmySessionBuffer& send_buff, int msg_id, const char* data, unsigned int len);
	int writeData(JmyDoubleSessionBuffer* write_buffer, int msg_id, const char* data, unsigned int len);

private:
	template <class SessionBuffer>
	int writeData(SessionBuffer* write_buffer, int msg_id, const char* data, unsigned int len);
	int processMsg(JmyMsgInfo*);
private:
	std::unordered_map<int, jmy_msg_handler> msg_handler_map_;
	JmyMsgInfo msg_info_;
};

template <class SessionBuffer>
int JmyDataHandler::processData(SessionBuffer* recv_buffer, int session_id, std::shared_ptr<JmyTcpSessionMgr> session_mgr)
{
	if (!recv_buffer) return -1;
	char* buff = recv_buffer->getReadBuff();
	unsigned int len = recv_buffer->getReadLen();

	unsigned int nhandled = 0;
	while (true) {
		if (len-nhandled < 2) {
			recv_buffer->readLen(nhandled);
			recv_buffer->moveDataToFront();
			std::cout << "JmyDataHandler::processData  not enough length to get header" << std::endl;
			break;
		}

		unsigned int data_len = ((buff[nhandled]<<8)&0xff00) + (buff[nhandled+1]&0xff);
		// current buffer not enough to hold next message data, switch to large buffer
		if (!recv_buffer->isLarge() && data_len > recv_buffer->getTotalLen()-2) {
			if (!recv_buffer->switchToLarge()) {
				std::cout << "JmyDataHandler::processData  current buffer size(" << recv_buffer->getTotalLen() << ") not enough to hold next message data(" << data_len << "), and cant malloc new large buffer" << std::endl;
				return -1;
			}
			if (data_len > recv_buffer->getTotalLen()-2) {
				std::cout << "JmyDataHandler::processData  next message data size(" << data_len << ") is too large than max buffer size(" << recv_buffer->getTotalLen()-2 << ")" << std::endl;
				return -1;
			}
			break;
		}

		// next message length small than normal buffer size, switch to normal buffer
		if (recv_buffer->isLarge() && data_len <= recv_buffer->getTotalLen()-2) {
			if (!recv_buffer->backToNormal()) {
				std::cout << "JmyDataHandler::processData  back to normal buffer failed" << std::endl;
				return -1;
			}
		}

		if (len-nhandled-2 < data_len) {
			recv_buffer->readLen(nhandled);
			std::cout << "JmyDataHandler::processData  not enough length to get data. data_len: " << data_len << ", left_len: " << len-nhandled-2 << std::endl;
			break;
		}

		int msg_id = ((buff[nhandled+2]<<8)&0xff00) + (buff[nhandled+3]&0xff);
		msg_info_.msg_id = msg_id;
		msg_info_.data = buff+nhandled;
		msg_info_.len = data_len - 2;
		msg_info_.session_id = session_id;
		msg_info_.param = (void*)(session_mgr.get());
		int res = processMsg(&msg_info_); 
		if (res < 0) return res;
		nhandled += (data_len+2);
		if (len - nhandled == 0) {
			break;
		}
	}
	return nhandled;
}

template <class SessionBuffer>
int JmyDataHandler::writeData(SessionBuffer* send_buffer, int msg_id, const char* data, unsigned int len)
{
	// write head
	char buf[2];
	buf[0] = ((len+2)>>8) & 0xff;
	buf[1] = (len+2)&0xff;
	if (!send_buffer->writeData(buf, 2)) {
		std::cout << "JmyDataHandler::writeData  write head failed" << std::endl;
		return -1;
	}
	// write msg_id
	buf[0] = (msg_id>>8)&0xff;
	buf[1] = msg_id&0xff;
	if (!send_buffer->writeData(buf, 2)) {
		std::cout << "JmyDataHandler::writeData  write msg_id failed" << std::endl;
		return -1;
	}
	// write body
	if (!send_buffer->writeData(data, len)) {
		std::cout << "JmyDataHandler::writeData  write data failed" << std::endl;
		return -1;
	}
	return len;
}
