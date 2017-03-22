#pragma once

#include "../libjmy/jmy_datatype.h"
#include "test_msg_handler.h"

#define s_libjmy_log_cate "libjmy_log"
#define s_client_log_cate "client_log"

static JmyId2MsgHandler s_test_handlers[] = {
	{ 1, TestMsgHandler::process_one },
	{ 2, TestMsgHandler::process_one }
};

#if USE_CONNECTOR_AND_SESSION
static JmyMultiConnectorsConfig test_connector_config = {
	{ 1024, 1024, s_test_handlers, sizeof(s_test_handlers)/sizeof(s_test_handlers[0]), true, true, true, { 100, 10 } }, 1000,
};
#else
static JmyClientsConfig test_clients_config = {
	{
		{ 1024, 1024, 0, 0, false, true },
		{ 10000, 10 },
		s_test_handlers,
		(sizeof(s_test_handlers)/sizeof(s_test_handlers[0])),
		false
	},
	(char*)"127.0.0.1",
	10000,
	5000,
};
#endif
