#pragma once

#include "../../proto/src/common.pb.h"
#include "../../proto/src/server.pb.h"
#include "../libjmy/jmy_datatype.h"
#include "conn_handler.h"

static JmyId2MsgHandler s_conn_handlers[] = {
	{ MSGID_LS2CS_CONNECT_REQUEST, ConnHandler::processLoginConnect },
	{ MSGID_GT2CS_CONNECT_REQUEST, ConnHandler::processGateConnect },
};
static JmyBaseEventHandlers s_conn_base_event_handlers = {
	ConnHandler::onConnect,
	ConnHandler::onDisconnect,
	ConnHandler::onTick,
	ConnHandler::onTimer
};
static JmyServerConfig s_conn_config = {
	{
		{ 4096, 4096, 0, 0, false, true},
		{ 10000, 10 },
		s_conn_handlers,
		sizeof(s_conn_handlers)/sizeof(s_conn_handlers[0]),
		s_conn_base_event_handlers,
		nullptr, 0,
		true
	},
	(char*)"127.0.0.1",
	10000,
	1024*10,
};
