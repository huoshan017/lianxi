#pragma once

#include "../../proto/src/common.pb.h"
#include "../../proto/src/server.pb.h"
#include "../libjmy/jmy_datatype.h"
#include "client_handler.h"
#include "game_handler.h"
#include "conn_login_handler.h"
#include "conn_config_handler.h"

static JmyRetransmissionConfig s_retran_config = {
	RETRANSMISSION_MAX_CACHED_SEND_BUFFER_COUNT,
	RETRANSMISSION_ACK_RECV_COUNT
};
// handle client config
static JmyId2MsgHandler s_client_handlers[] = {
	{ MSGID_C2S_ENTER_GAME_REQUEST, ClientHandler::processEnterGame },
	{ MSGID_C2S_RECONNECT_REQUEST, ClientHandler::processReconnect}
};
static jmy_msg_handler s_default_client_handler = nullptr;
static JmyBaseEventHandlers s_client_base_event_handlers = {
	ClientHandler::onConnect,
	ClientHandler::onDisconnect,
	ClientHandler::onTick,
	ClientHandler::onTimer
};
static JmyConnectionConfig s_client_connection_config = {
	{ 2048, 2048, 0, 0, false, true},
	&s_retran_config,
	s_client_handlers,
	sizeof(s_client_handlers)/sizeof(s_client_handlers[0]),
	s_default_client_handler,
	s_client_base_event_handlers,
	nullptr, 0,
	true
};
static JmyServerConfig s_client_config = {
	s_client_connection_config,
	(char*)"127.0.0.1",
	10000,
	1024*10
};

// handle game_server config
static JmyId2MsgHandler s_game_handlers[] = {
	{ MSGID_GS2GT_CONNECT_GATE_REQUEST, GameHandler::processConnectGateRequest },
};
static JmyBaseEventHandlers s_game_base_event_handlers = {
	GameHandler::onConnect,
	GameHandler::onDisconnect,
	GameHandler::onTick,
	GameHandler::onTimer
};
static JmyConnectionConfig s_game_connection_config = {
	{ 2048*100, 2048*100, 0, 0, false, true },
	&s_retran_config,
	s_game_handlers,
	sizeof(s_game_handlers)/sizeof(s_game_handlers[0]),
	(jmy_msg_handler)nullptr,
	s_game_base_event_handlers,
	nullptr, 0,
	true
};
static JmyServerConfig s_game_config = {
	s_game_connection_config,
	(char*)"127.0.0.1", // ip
	0, // port
	10 // max_conn
};

// handle connection to config_server config
static JmyId2MsgHandler s_config_handlers[] = {
	{ MSGID_CS2GT_CONNECT_CONFIG_RESPONSE, ConnConfigHandler::processConnectConfigResponse },
	{ MSGID_CS2GT_NEW_LOGIN_NOTIFY, ConnConfigHandler::processNewLoginNotify },
	{ MSGID_CS2GT_REMOVE_LOGIN_NOTIFY, ConnConfigHandler::processRemoveLoginNotify }
};
static JmyBaseEventHandlers s_config_base_event_handlers = {
	ConnConfigHandler::onConnect,
	ConnConfigHandler::onDisconnect,
	ConnConfigHandler::onTick,
	ConnConfigHandler::onTimer,
};
static JmyConnectionConfig s_config_conn_config = {
	{ 1024*64, 1024*64, 0, 0, false, true },
	&s_retran_config,
	s_config_handlers,
	sizeof(s_config_handlers)/sizeof(s_config_handlers[0]),
	(jmy_msg_handler)nullptr,
	s_config_base_event_handlers,
	(JmyId2EventHandler*)nullptr, 0,
	true
};
static JmyClientConfig s_config_config = {
	s_config_conn_config,
	(char*)"0.0.0.0", 0,
	true
};

// handle connection to login_server config
static JmyId2MsgHandler s_login_handlers[] = {
	{ MSGID_LS2GT_CONNECT_LOGIN_RESPONSE, ConnLoginHandler::processConnectLoginResponse },
	{ MSGID_LS2GT_SELECTED_SERVER_NOTIFY, ConnLoginHandler::processSelectedServerNotify }
};
static JmyBaseEventHandlers s_login_base_event_handlers = {
	ConnLoginHandler::onConnect,
	ConnLoginHandler::onDisconnect,
	ConnLoginHandler::onTick,
	ConnLoginHandler::onTimer,
};
static JmyConnectionConfig s_login_conn_config = {
	{ 1024*64, 1024*64, 0, 0, false, true },
	&s_retran_config,
	s_login_handlers,
	sizeof(s_login_handlers)/sizeof(s_login_handlers[0]),
	(jmy_msg_handler)nullptr,
	s_login_base_event_handlers,
	(JmyId2EventHandler*)nullptr, 0,
	true
};
static JmyClientConfig s_login_config = {
	s_login_conn_config,
	(char*)"0.0.0.0", 20000,
	true,
};
