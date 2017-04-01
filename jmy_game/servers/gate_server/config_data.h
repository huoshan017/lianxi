#pragma once

#include "../../proto/src/common.pb.h"
#include "../../proto/src/server.pb.h"
#include "../libjmy/jmy_datatype.h"
#include "client_msg_handler.h"
#include "login_msg_handler.h"

// handle client config
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

// handle game_server config
static JmyId2MsgHandler s_game_handlers[] = {
};
static JmyServerConfig s_game_config = {
	{
		{ 2048*100, 2048*100, 0, 0, false, true},
		{ 10000, 10 },
		s_game_handlers,
		sizeof(s_game_handlers)/sizeof(s_game_handlers[0]),
		true
	},
	(char*)"127.0.0.1", // ip
	0, // port
	10 // max_conn
};

// handle connection to login_server config
static JmyId2MsgHandler s_login_handlers[] = {
};
static JmyConnectionConfig s_login_config = {
	{ 1024*64, 1024*64, 0, 0, false, true },
	{ 1000, 10 },
	s_login_handlers,
	sizeof(s_login_handlers)/sizeof(s_login_handlers[0]),
	true
};
