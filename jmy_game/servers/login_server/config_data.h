#pragma once

#include "../../proto/src/common.pb.h"
#include "../../proto/src/server.pb.h"
#include "../libjmy/jmy_datatype.h"
#include "client_handler.h"
#include "gate_handler.h"
#include "conn_config_handler.h"

static JmyResendConfig s_retran_config = {
	RETRANSMISSION_MAX_CACHED_SEND_BUFFER_COUNT,
	RETRANSMISSION_ACK_RECV_COUNT
};

/* client handler config */
static JmyId2MsgHandler s_client_handlers[] = {
	{ MSGID_C2S_LOGIN_REQUEST, ClientHandler::processLogin },
	{ MSGID_C2S_SELECT_SERVER_REQUEST, ClientHandler::processSelectServer },
};
static JmyBaseEventHandlers s_client_base_event_handlers = {
	ClientHandler::onConnect,
	ClientHandler::onDisconnect,
	ClientHandler::onTick,
	ClientHandler::onTimer,
};
static JmyConnectionConfig s_client_conn_config = {
	{ 2048, 2048, 0, 0, false, true },
	(JmyResendConfig*)nullptr, //&s_retran_config,
	s_client_handlers,
	sizeof(s_client_handlers)/sizeof(s_client_handlers[0]),
	(jmy_msg_handler)nullptr,
	s_client_base_event_handlers,
	(JmyId2EventHandler*)nullptr, 0,
	true
};
static JmyServerConfig s_client_config = {
	s_client_conn_config,
	(char*)"127.0.0.1",
	10000,
	1024*10
};


/* gate handler config */
static JmyId2MsgHandler s_gate_handlers[] = {
	{ MSGID_GT2LS_CONNECT_LOGIN_REQUEST, GateHandler::processConnectLogin },
	{ MSGID_GT2LS_SELECTED_SERVER_RESPONSE, GateHandler::processSelectedServerResponse },
};
static JmyBaseEventHandlers s_gate_base_event_handlers = {
	GateHandler::onConnect,
	GateHandler::onDisconnect,
	GateHandler::onTick,
	GateHandler::onTimer,
};
static JmyConnectionConfig s_gate_conn_config = {
	{ 1024*64, 1024*64, 0, 0, false, true },
	&s_retran_config,
	s_gate_handlers,
	sizeof(s_gate_handlers)/sizeof(s_gate_handlers[0]),
	(jmy_msg_handler)nullptr,
	s_gate_base_event_handlers,
	nullptr, 0,
	true
};
static JmyServerConfig s_gate_config = {
	s_gate_conn_config,
	(char*)"0.0.0.0",
	20000,
	4
};

/* connect config handler config */
static JmyId2MsgHandler s_conn_config_handlers[] = {
	{ MSGID_CS2LS_CONNECT_CONFIG_RESPONSE, ConnConfigHandler::processConnectConfigResponse },
};
static JmyBaseEventHandlers s_conn_config_base_event_handlers = {
	ConnConfigHandler::onConnect,
	ConnConfigHandler::onDisconnect,
	ConnConfigHandler::onTick,
	ConnConfigHandler::onTimer,
};
static JmyConnectionConfig s_conn_conn_config = {
	{ 1024*64, 1024*64, 0, 0, false, true },
	&s_retran_config,
	s_conn_config_handlers,
	sizeof(s_conn_config_handlers)/sizeof(s_conn_config_handlers[0]),
	(jmy_msg_handler)nullptr,
	s_conn_config_base_event_handlers,
	nullptr, 0,
	true
};
static JmyClientConfig s_conn_config = {
	s_conn_conn_config,
	true,
	CLIENT_RECONNECT_INTERVAL_SECS,
};
