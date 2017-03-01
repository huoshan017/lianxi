#include "test_msg_handler.h"
#include "../net_tcp/jmy_tcp_connector.h"
#include <iostream>

int TestMsgHandler::process_one(JmyMsgInfo* info)
{
	if (!info) return -1;
	const char* data = info->data;
	unsigned int len = info->len;
	JmyTcpConnector* connector = (JmyTcpConnector*)info->param;
	std::cout << "TestMsgHandler::process_one: data(" << data << "), len(" << len << ")" << std::endl;
	if (!connector) {
		std::cout << "error  TestMsgHandler::process_one: connector not found" << std::endl;
		return -1;
	}
	connector->send(1, data, len);
	return len;
}
