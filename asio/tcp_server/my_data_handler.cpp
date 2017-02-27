#include "my_data_handler.h"
#include "my_tcp_session.h"
#include <iostream>

MyDataHandler::MyDataHandler()
{
}

MyDataHandler::~MyDataHandler()
{
}

bool MyDataHandler::loadMsgHandle(const MyId2MsgHandler id2handlers[], int size)
{
	if (!id2handlers || size == 0)
		return false;

	int i = 0;
	for (; i<size; ++i) {
		msg_handler_map_.insert(std::make_pair(id2handlers[i].msg_id, id2handlers[i].handler));
	}

	return true;
}

int MyDataHandler::processData(MySessionBuffer* read_buffer, int session_id)
{
	if (!read_buffer) return -1;
	char* buff = read_buffer->getReadBuff();
	unsigned int len = read_buffer->getReadLen();

	unsigned int nhandled = 0;
	while (true) {
		if (len-nhandled < 2) {
			read_buffer->readLen(nhandled);
			read_buffer->moveDataToFront();
			std::cout << "not enough length to get header" << std::endl;
			break;
		}

		unsigned int data_len = ((buff[nhandled]<<8)&0xff00) + (buff[nhandled+1]&0xff);
		if (data_len > read_buffer->getTotalLen()) {
		
		}
		if (len-nhandled-2 < data_len) {
			read_buffer->readLen(nhandled);
			std::cout << "not enough length to get data" << std::endl;
			break;
		}

		int msg_id = ((buff[nhandled+2]<<8)&0xff00) + (buff[nhandled+3]&0xff);
		int res = processMsg(msg_id, buff+nhandled, data_len-2, session_id);
		if (res < 0) return res;
		nhandled += res;
		if (len - nhandled == 0) {
			break;
		}
	}
	return nhandled;
}

int MyDataHandler::writeData(MySessionBuffer* buffer, const char* data, unsigned int len)
{
	if (!buffer || !data || !len)
		return 0;

	if (!buffer->checkWriteLen(len+2)) {
		std::cout << "MyDataHandler::writeData checkWriteLen(" << len+2 << ") failed" << std::endl;
		return -1;
	}
	// write head
	char head[2];
	head[0] = (len>>8) & 0xff;
	head[1] = len&0xff;
	if (!buffer->writeData(head, 2)) {
		std::cout << "MyDataHandler::writeData write head failed" << std::endl;
		return -1;
	}
	if (!buffer->writeData(data, len)) {
		std::cout << "MyDataHandler::writeData write data failed" << std::endl;
		return -1;
	}
	return len;
}

int MyDataHandler::processMsg(int msg_id, const char* data, unsigned int len, int session_id)
{
	std::unordered_map<int, my_msg_handler>::iterator it = msg_handler_map_.find(msg_id);
	if (it == msg_handler_map_.end()) {
		std::cout << "not found msg(" << msg_id << ") handler, session_id(" << session_id << ")" << std::endl;
		return len;
	}
	if (it->second(data, len, session_id) < 0)
		return -1;
	return len;
}
