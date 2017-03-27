#pragma once

#include "../libjmy/jmy_datatype.h"
#include "login_msg_handler.h"

static JmyId2MsgHandler s_login_handlers[] = {
	{ 1, LoginMsgHandler::processLogin },
	{ 2, LoginMsgHandler::processSelectGameServer },
	{ 3, LoginMsgHandler::processEnterGameServer },
};

static JmyServerConfig s_login_config = {
	{
		{ 2048, 2048, 0, 0, false, true},
		{ 10000, 10 },
		s_login_handlers,
		sizeof(s_login_handlers)/sizeof(s_login_handlers[0]),
		true
	},
	(char*)"127.0.0.1",
	10000,
	1024*10
};
