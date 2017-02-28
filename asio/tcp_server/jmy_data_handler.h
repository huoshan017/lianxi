#pragma once

#include <unordered_map>
#include "jmy_datatype.h"
#include "jmy_session_buffer.h"
#include "jmy_session_buffer_pool.h"
#include "jmy_tcp_session.h"
#include <iostream>

class JmyDataHandler
{
public:
	JmyDataHandler();
	~JmyDataHandler();
	bool loadMsgHandle(const JmyId2MsgHandler id2handlers[], int size);
	template <class SessionBuffer>
	int processData(SessionBuffer* recv_buffer, int session_id, std::shared_ptr<JmyTcpSessionMgr> session_mgr);
	template <class SessionBuffer>
	int writeData(SessionBuffer* write_buffer, const char* data, unsigned int len);

private:
	int processMsg(int msg_id, const char* data, unsigned int len, int session_id, std::shared_ptr<JmyTcpSessionMgr> session_mgr);
private:
	std::unordered_map<int, jmy_msg_handler> msg_handler_map_;
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
			std::cout << "MyDataHandler::processData  not enough length to get header" << std::endl;
			break;
		}

		unsigned int data_len = ((buff[nhandled]<<8)&0xff00) + (buff[nhandled+1]&0xff);
		// current buffer not enough to hold next message data, switch to large buffer
		if (!recv_buffer->isLarge() && data_len > recv_buffer->getTotalLen()-2) {
			if (!recv_buffer->switchToLarge()) {
				std::cout << "MyDataHandler::processData  current buffer size(" << recv_buffer->getTotalLen() << ") not enough to hold next message data(" << data_len << "), and cant malloc new large buffer" << std::endl;
				return -1;
			}
			if (data_len > recv_buffer->getTotalLen()-2) {
				std::cout << "MyDataHandler::processData  next message data size(" << data_len << ") is too large than max buffer size(" << recv_buffer->getTotalLen()-2 << ")" << std::endl;
				return -1;
			}
			break;
		}

		// next message length small than normal buffer size, switch to normal buffer
		if (recv_buffer->isLarge() && data_len <= recv_buffer->getTotalLen()-2) {
			if (!recv_buffer->backToNormal()) {
				std::cout << "MyDataHandler::processData  back to normal buffer failed" << std::endl;
				return -1;
			}
		}

		if (len-nhandled-2 < data_len) {
			recv_buffer->readLen(nhandled);
			std::cout << "MyDataHandler::processData  not enough length to get data. datalen: " << data_len << ", leftlen: " << len-nhandled-2 << std::endl;
			break;
		}

		int msg_id = ((buff[nhandled+2]<<8)&0xff00) + (buff[nhandled+3]&0xff);
		int res = processMsg(msg_id, buff+nhandled, data_len-2, session_id, session_mgr);
		if (res < 0) return res;
		nhandled += res;
		if (len - nhandled == 0) {
			break;
		}
	}
	return nhandled;
}

template <class SessionBuffer>
int JmyDataHandler::writeData(SessionBuffer* send_buffer, const char* data, unsigned int len)
{
	if (!send_buffer || !data || !len)
		return 0;

	if (!send_buffer->checkWriteLen(len+2)) {
		bool too_large = true;
		if (!send_buffer->isLarge()) {
			if (!send_buffer->switchToLarge()) {
				std::cout << "MyDataHandler::writeData  switch to large buffer failed" << std::endl;
				return -1;
			}
			if (send_buffer->checkWriteLen(len+2)) {
				too_large = false;
			}
		}
		if (too_large) {
			std::cout << "MyDataHandler::writeData  data length(" << len << ") is too large to write" << std::endl;
			return -1;
		}
	}
	// check if length of data is small than normal buffer, switch to normal buffer
	if (send_buffer->isLarge() && send_buffer->getNormalLen() >= len+2)  {
		if (!send_buffer->backToNormal()) {
			std::cout << "MyDataHandler::writeData  back to normal buffer failed" << std::endl;
			return -1;
		}
	}
	// write head
	char head[2];
	head[0] = (len>>8) & 0xff00;
	head[1] = len&0xff;
	if (!send_buffer->writeData(head, 2)) {
		std::cout << "MyDataHandler::writeData write head failed" << std::endl;
		return -1;
	}
	// write body
	if (!send_buffer->writeData(data, len)) {
		std::cout << "MyDataHandler::writeData write data failed" << std::endl;
		return -1;
	}
	return len;
}
