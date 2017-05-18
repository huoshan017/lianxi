#pragma once

#include "../../proto/src/common.pb.h"
#include "../../proto/src/server.pb.h"
#include "../libjmy/jmy_datatype.h"
#include "client_handler.h"
#include "game_handler.h"
#include "conn_login_handler.h"
#include "conn_config_handler.h"

static JmyResendConfig s_retran_config = {
	RESEND_MAX_CACHED_SEND_BUFFER_COUNT,
	RESEND_ACK_RECV_COUNT
};
// handle client config
static JmyId2MsgHandler s_client_handlers[] = {
	{ MSGID_C2S_GET_ROLE_REQUEST,	ClientHandler::processGetRoleRequest },  // handle role list request
	{ MSGID_C2S_CREATE_ROLE_REQUEST,ClientHandler::processCreateRoleRequest },// handle create role request
	//{ MSGID_C2S_DELETE_ROLE_REQUEST,ClientHandler::processDeleteRoleRequest },// handle delete role request
	{ MSGID_C2S_ENTER_GAME_REQUEST, ClientHandler::processEnterGameRequest }, // handle enter game request
	{ MSGID_C2S_LEAVE_GAME_REQUEST, ClientHandler::processLeaveGameRequest }, // handle leave game request
	{ MSGID_C2S_RECONNECT_REQUEST,	ClientHandler::processReconnectRequest }  // handle reconnect request
};
static jmy_msg_handler s_default_client_handler = ClientHandler::processDefault;
static JmyBaseEventHandlers s_client_base_event_handlers = {
	ClientHandler::onConnect,
	ClientHandler::onDisconnect,
	ClientHandler::onTick,
	nullptr,
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
	{ MSGID_GS2GT_ENTER_GAME_RESPONSE, GameHandler::processEnterGameResponse },
	{ MSGID_GS2GT_LEAVE_GAME_RESPONSE, GameHandler::processLeaveGameResponse }
};
static jmy_msg_handler s_default_game_handler = GameHandler::processDefault;
static JmyBaseEventHandlers s_game_base_event_handlers = {
	GameHandler::onConnect,
	GameHandler::onDisconnect,
	GameHandler::onTick,
	nullptr,
};
static JmyConnectionConfig s_game_connection_config = {
	{ 2048*100, 2048*100, 0, 0, false, true },
	&s_retran_config,
	s_game_handlers,
	sizeof(s_game_handlers)/sizeof(s_game_handlers[0]),
	s_default_game_handler,
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
	nullptr,
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
	true,
	CLIENT_RECONNECT_INTERVAL_SECS,
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
	nullptr,
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
	true,
	CLIENT_RECONNECT_INTERVAL_SECS,
};
