#pragma once

#include "../../proto/src/common.pb.h"
#include "../libjmy/jmy_datatype.h"
#include "../common/defines.h"
#include "login_handler.h"
#include "game_handler.h"

/* connect login handler config */
static JmyResendConfig s_retran_config = {
	RETRANSMISSION_MAX_CACHED_SEND_BUFFER_COUNT,
	RETRANSMISSION_ACK_RECV_COUNT
};
static JmyId2MsgHandler s_login_handlers[] = {
	{ MSGID_S2C_LOGIN_RESPONSE, LoginHandler::processLogin },
	{ MSGID_S2C_SELECT_SERVER_RESPONSE, LoginHandler::processSelectedServer }
};
static JmyBaseEventHandlers s_login_base_event_handlers = {
	LoginHandler::onConnect,
	LoginHandler::onDisconnect,
	LoginHandler::onTick,
	nullptr,
};
static jmy_msg_handler s_login_default_msg_handler = LoginHandler::processDefault;
static JmyConnectionConfig s_conn_login_config = {
	{ 1024, 1024, 0, 0, false, true },
	&s_retran_config,
	s_login_handlers,
	sizeof(s_login_handlers)/sizeof(s_login_handlers[0]),
	s_login_default_msg_handler,
	s_login_base_event_handlers,
	nullptr, 0,
	true
};
static JmyClientConfig s_login_config = {
	s_conn_login_config,
	true,
	CLIENT_RECONNECT_INTERVAL_SECS,
};

/* connect game handler config */
static JmyId2MsgHandler s_game_handlers[] = {
	{ MSGID_C2S_ENTER_GAME_REQUEST, GameHandler::processEnterGame },
	{ MSGID_C2S_RECONNECT_REQUEST, GameHandler::processReconnect }
};
static JmyBaseEventHandlers s_game_base_event_handlers = {
	GameHandler::onConnect,
	GameHandler::onDisconnect,
	GameHandler::onTick,
	nullptr,
};
static jmy_msg_handler s_game_default_msg_handler = GameHandler::processDefault;
static JmyConnectionConfig s_conn_game_config = {
	{ 1024*128, 1024*128, 0, 0, false, true },
	&s_retran_config,
	s_game_handlers,
	sizeof(s_game_handlers)/sizeof(s_game_handlers[0]),
	s_game_default_msg_handler,
	s_game_base_event_handlers,
	nullptr, 0,
	true
};
static JmyClientConfig s_game_config = {
	s_conn_game_config,
	true,
	CLIENT_RECONNECT_INTERVAL_SECS,
};
