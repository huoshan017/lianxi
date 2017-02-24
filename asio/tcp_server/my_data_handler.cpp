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

int MyDataHandler::processData(const char* data, unsigned int len, int session_id)
{
	if (!data) return -1;

	int nhandled = 0;
	char* p = (char*)data;
	while (true) {
		if (len-nhandled < 2) {
			std::cout << "not enough length to get header" << std::endl;
			break;
		}

		unsigned int data_len = ((p[nhandled]<<8)&0xff) + (p[nhandled+1]&0xff);
		if (len-nhandled-2 < data_len) {
			std::cout << "not enough length to get data" << std::endl;
			break;
		}

		int msg_id = ((p[nhandled+2]<<8)&0xff) + (p[nhandled+3]&0xff);
		int res = processMsg(msg_id, data+nhandled, data_len-2, session_id);
		if (res < 0) return res;
		nhandled += res;
		if (len - nhandled == 0) {
			break;
		}
	}
	return nhandled;
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
