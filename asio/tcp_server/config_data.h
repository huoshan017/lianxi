#pragma once

#include "../libjmy/jmy_datatype.h"
#include "test_msg_handler.h"

#define s_libjmy_log_cate "libjmy_log"
#define s_server_log_cate "server_log"

static JmyId2MsgHandler s_test_handlers[] = {
	{ 1, TestMsgHandler::process_one }
};

static JmyServerConfig test_config = {
	{ 1024, 1024, 1024, 1024, true, { { 1, 100000 }, 100, 10 } },
	s_test_handlers,
	sizeof(s_test_handlers)/sizeof(s_test_handlers[0]),
	1024*10
};

