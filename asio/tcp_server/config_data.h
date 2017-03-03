#include "../libjmy/jmy_datatype.h"
#include "test_msg_handler.h"

JmyId2MsgHandler s_test_handlers[] = {
	{ 1, TestMsgHandler::process_one }
};

const JmyServerConfig test_config = {
	{1024, 1024, 1024, 1024},
	s_test_handlers,
	sizeof(s_test_handlers)/sizeof(s_test_handlers[0]),
	1024*10
};

