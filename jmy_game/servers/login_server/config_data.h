#pragma once

#include "../../proto/common.pb.h"
#include "../libjmy/jmy_datatype.h"
#include "login_msg_handler.h"

static JmyId2MsgHandler s_login_handlers[] = {
	{ MSGID_C2L_LOGIN_REQUEST, LoginMsgHandler::processLogin },
	{ MSGID_C2L_SELECT_SERVER_REQUEST, LoginMsgHandler::processSelectServer },
	{ MSGID_C2G_ENTER_GAME_REQUEST, LoginMsgHandler::processEnterGame },
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
