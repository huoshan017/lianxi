#pragma once

#include "../../proto/src/common.pb.h"
#include "../../proto/src/server.pb.h"
#include "../libjmy/jmy_datatype.h"
#include "../common/defines.h"
#include "game_handler.h"

static JmyRetransmissionConfig s_retran_config = {
	RETRANSMISSION_MAX_CACHED_SEND_BUFFER_COUNT,
	RETRANSMISSION_ACK_RECV_COUNT
};
static JmyId2MsgHandler s_game_handlers[] = {
};
static JmyBaseEventHandlers s_game_base_event_handlers = {
	GameHandler::onConnect,
	GameHandler::onDisconnect,
	GameHandler::onTick,
	nullptr,
};
static JmyConnectionConfig s_game_conn_config = {
	{ 4096, 4096, 0, 0, false, true},	// JmyBufferConfig
	&s_retran_config,					// JmyRetransmissionConfig
	s_game_handlers,					// JmyId2MsgHandler []
	sizeof(s_game_handlers)/sizeof(s_game_handlers[0]), // int
	(jmy_msg_handler)nullptr,			// jmy_msg_handler
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
