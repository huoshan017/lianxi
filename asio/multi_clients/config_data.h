#pragma once

#include "../libjmy/jmy_datatype.h"
#include "test_msg_handler.h"

#define s_libjmy_log_cate "libjmy_log"
#define s_client_log_cate "client_log"

static JmyId2MsgHandler s_test_handlers[] = {
	{ 1, TestMsgHandler::process_one }
};

static JmyMultiConnectorsConfig test_connector_config = {
	{ 1024, 1024, s_test_handlers, sizeof(s_test_handlers)/sizeof(s_test_handlers), true, true, true }, 1000,
};