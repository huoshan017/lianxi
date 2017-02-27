#include "my_datatype.h"
#include "test_msg_handler.h"

MyId2MsgHandler s_test_handlers[] = {
	{ 1, TestMsgHandler::process_one }
};

const MyServerConfig test_config = {
	{1024, 1024, 1024, 1024},
	s_test_handlers,
	sizeof(s_test_handlers)/sizeof(s_test_handlers[0]),
	1024*10
};
