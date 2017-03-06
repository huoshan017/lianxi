#include "test_msg_handler.h"
#include "../libjmy/jmy_tcp_connector.h"
#include <iostream>
#include "const_data.h"

int TestMsgHandler::count_ = 0;

int TestMsgHandler::process_one(JmyMsgInfo* info)
{
	if (!info) return -1;
	const char* data = info->data;
	unsigned int len = info->len;
	JmyTcpConnector* connector = (JmyTcpConnector*)info->param;
	int s = sizeof(s_send_data)/sizeof(s_send_data[0]);
	int index = get_count() % s;
	if (std::memcmp(data, s_send_data[index], len) != 0) {
		std::cout << "TestMsgHandler::process_one  get data from msg(" << data << ") compared from s_send_data["<< index << "] (" << s_send_data[index] << ") is different" << std::endl;
	} else {
		//std::cout << "TestMsgHandler::process_one  compare data(" << data << ") is same" << std::endl;
	}
	inc_count();

	static int count = 0;
	count += 1;

	if (!connector) {
		std::cout << "error  TestMsgHandler::process_one: connector not found" << std::endl;
		return -1;
	}
	//connector->send(1, data, len);
	return len;
}
