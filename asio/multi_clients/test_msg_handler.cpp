#include "test_msg_handler.h"
#include "../libjmy/jmy_tcp_connector.h"
#include "const_data.h"
#include "util.h"

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
		ClientLogError("TestMsgHandler::process_one  get data from msg(%s) compared from s_send_data[%d] (%s) is different", data, index, s_send_data[index]);
	} else {
		//std::cout << "TestMsgHandler::process_one  compare data(" << data << ") is same" << std::endl;
	}
	inc_count();

	static int count = 0;
	count += 1;

	if (!connector) {
		ClientLogError("error  TestMsgHandler::process_one: connector not found");
		return -1;
	}
	return len;
}
