#pragma once

#include "../../proto/src/common.pb.h"
#include "../../proto/src/server.pb.h"
#include "../libjmy/jmy_datatype.h"
#include "client_msg_handler.h"
#include "login_msg_handler.h"

static JmyId2MsgHandler s_client_handlers[] = {
};

static JmyServerConfig s_client_config = {
	{
		{ 2048, 2048, 0, 0, false, true},
		{ 10000, 10 },
		s_client_handlers,
		sizeof(s_client_handlers)/sizeof(s_client_handlers[0]),
		true
	},
	(char*)"127.0.0.1",
	10000,
	1024*10
};

static JmyId2MsgHandler s_login_handlers[] = {
};

static JmyServerConfig s_login_config = {
	{
		{ 1024*64, 1024*64, 0, 0, false, true },
		{ 1000, 10 },
		s_login_handlers,
		sizeof(s_login_handlers)/sizeof(s_login_handlers[0]),
		true
	},
	(char*)"0.0.0.0",
	20000,
	4
};
