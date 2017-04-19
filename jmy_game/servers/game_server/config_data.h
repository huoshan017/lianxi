#pragma once
#include "../../proto/src/common.pb.h"
#include "../../proto/src/server.pb.h"
#include "../libjmy/jmy_datatype.h"
#include "../common/defines.h"
#include "conn_gate_handler.h"
#include "conn_db_handler.h"

/* connect config handler config */
static JmyResendConfig s_retran_config = {
	RETRANSMISSION_MAX_CACHED_SEND_BUFFER_COUNT,
	RETRANSMISSION_ACK_RECV_COUNT
};
static JmyId2MsgHandler s_conn_gate_handlers[] = {
	{ MSGID_GT2GS_CONNECT_GATE_RESPONSE, ConnGateHandler::processConnectGateResponse },
	{ MSGID_GT2GS_ENTER_GAME_REQUEST, ConnGateHandler::processEnterGame },
	{ MSGID_GT2GS_LEAVE_GAME_REQUEST, ConnGateHandler::processLeaveGame }
};
static JmyBaseEventHandlers s_conn_gate_base_event_handlers = {
	ConnGateHandler::onConnect,
	ConnGateHandler::onDisconnect,
	ConnGateHandler::onTick,
	ConnGateHandler::onTimer,
};
static jmy_msg_handler s_conn_gate_default_msg_handler = ConnGateHandler::processDefault;
static JmyConnectionConfig s_conn_gate_config = {
	{ 1024*64, 1024*64, 0, 0, false, true },
	&s_retran_config,
	s_conn_gate_handlers,
	sizeof(s_conn_gate_handlers)/sizeof(s_conn_gate_handlers[0]),
	s_conn_gate_default_msg_handler,
	s_conn_gate_base_event_handlers,
	nullptr, 0,
	true
};
static JmyClientConfig s_gate_config = {
	s_conn_gate_config,
	true,
	CLIENT_RECONNECT_INTERVAL_SECS,
};


/* conn db_server configures */
static JmyId2MsgHandler s_conn_db_handlers[] = {
	{ MSGID_DS2GS_CONNECT_DB_RESPONSE, ConnDBHandler::processConnectDBResponse },
	{ MSGID_DS2GS_CONNECT_DB_RESPONSE, ConnDBHandler::processConnectDBResponse }
};
static JmyBaseEventHandlers s_conn_db_base_event_handlers = {
	ConnDBHandler::onConnect,
	ConnDBHandler::onDisconnect,
	ConnDBHandler::onTick,
	nullptr,
};
static jmy_msg_handler s_conn_db_default_msg_handler = ConnDBHandler::processDefault;
static JmyConnectionConfig s_conn_db_config = {
	{ 1024*128, 1024*128, 0, 0, false, true },
	&s_retran_config,
	s_conn_db_handlers,
	sizeof(s_conn_db_handlers)/sizeof(s_conn_db_handlers[0]),
	s_conn_db_default_msg_handler,
	s_conn_db_base_event_handlers,
	nullptr, 0, true
};
static JmyClientConfig s_db_config = {
	s_conn_db_config,
	true,
	CLIENT_RECONNECT_INTERVAL_SECS,
};
