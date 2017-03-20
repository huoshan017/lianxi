#pragma once

#include "../libjmy/jmy_datatype.h"
#include "test_msg_handler.h"

#define s_libjmy_log_cate "libjmy_log"
#define s_server_log_cate "server_log"

static JmyId2MsgHandler s_test_handlers[] = {
	{ 1, TestMsgHandler::process_one }
};

static JmyServerConfig test_config = {
#if USE_CONNECTOR_AND_SESSION
	{ 1024, 1024, 1024, 1024, true, { 100, 10 } },
#else
	{
		{ 2048, 2048, 0, 0, false, true} ,
		{ 100, 20 },
		s_test_handlers,
		sizeof(s_test_handlers)/sizeof(s_test_handlers[0]),
		true
	},
#endif
	(char*)"127.0.0.1",
	10000,
	1024*10
};

