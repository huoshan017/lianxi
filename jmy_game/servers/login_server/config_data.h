#pragma once

#include "../../proto/src/common.pb.h"
#include "../../proto/src/server.pb.h"
#include "../libjmy/jmy_datatype.h"
#include "client_msg_handler.h"
#include "gate_msg_handler.h"

static JmyId2MsgHandler s_client_handlers[] = {
	{ MSGID_C2L_LOGIN_REQUEST, ClientMsgHandler::processLogin },
	{ MSGID_C2L_SELECT_SERVER_REQUEST, ClientMsgHandler::processSelectServer },
};

static JmyServerConfig s_login_config = {
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

static JmyId2MsgHandler s_gate_handlers[] = {
	{ MSGID_T2L_CONNECT_REQUEST, GateMsgHandler::processConnect },
	{ MSGID_T2L_SELECTED_SERVER_RESPONSE, GateMsgHandler::processSelectedServerResponse },
};

static JmyServerConfig s_gate_config = {
	{
		{ 1024*64, 1024*64, 0, 0, false, true },
		{ 1000, 10 },
		s_gate_handlers,
		sizeof(s_gate_handlers)/sizeof(s_gate_handlers[0]),
		true
	},
	(char*)"0.0.0.0",
	20000,
	4
};
