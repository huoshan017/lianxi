#include "test_msg_handler.h"
#include "../net_tcp/jmy_tcp_connector.h"
#include <iostream>
#include "const_data.h"

int TestMsgHandler::process_one(JmyMsgInfo* info)
{
	if (!info) return -1;
	const char* data = info->data;
	unsigned int len = info->len;
	JmyTcpConnector* connector = (JmyTcpConnector*)info->param;
	static int index = 0;
	int s = sizeof(s_send_data)/sizeof(s_send_data[0]);
	if (std::memcmp(data, s_send_data[index], len) != 0) {
		std::cout << "TestMsgHandler::process_one  get data from msg(" << data << ") compared from s_send_data["<< index << "] is different" << std::endl;
	} else {
		//std::cout << "TestMsgHandler::process_one  compare data is same" << std::endl;
	}
	index += 1;
	if (index >= s)
		index = 0;

	static int count = 0;
	count += 1;
	std::cout << "TestMsgHandler::process_one  processed " << count << " count" << std::endl;

	if (!connector) {
		std::cout << "error  TestMsgHandler::process_one: connector not found" << std::endl;
		return -1;
	}
	connector->send(1, data, len);
	return len;
}
