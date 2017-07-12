#pragma once

#include "../../proto/src/server.pb.h"
#include "../libjmy/jmy_datatype.h"
#include "../common/defines.h"
#include "game_handler.h"

static JmyId2MsgHandler s_game_handlers[] = {
	{ MSGID_GS2DS_CONNECT_DB_REQUEST, GameHandler::processConnectDBRequest },
	{ MSGID_GS2DS_GET_ROLE_REQUEST, GameHandler::processGetRole },
	{ MSGID_GS2DS_CREATE_ROLE_REQUEST, GameHandler::processCreateRole },
};
static JmyBaseEventHandlers s_game_base_event_handlers = {
	GameHandler::onConnect,
	GameHandler::onDisconnect,
	GameHandler::onTick,
	nullptr,
};

static jmy_msg_handler s_default_game_conn_handler = GameHandler::processDefault;
static JmyConnectionConfig s_game_conn_config = {
	{ 4096, 4096, 0, 0, false, true},	// JmyBufferConfig
	s_game_handlers,					// JmyId2MsgHandler []
	sizeof(s_game_handlers)/sizeof(s_game_handlers[0]), // int
	s_default_game_conn_handler,		// jmy_msg_handler
	s_game_base_event_handlers,			// JmyId2EventHandler []
	(JmyId2EventHandler*)nullptr, 0,	// JmyId2EventHandler*
	true								// is_reconnect : bool
};
static JmyServerConfig s_game_config = {
	s_game_conn_config,
	(char*)"127.0.0.1",
	10000,
	1024*10,
};
