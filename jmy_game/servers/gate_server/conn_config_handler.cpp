#include "conn_config_handler.h"
#include "../common/agent.h"
#include "../common/util.h"

char ConnConfigHandler::tmp_[MAX_SEND_BUFFER_SIZE];

int ConnConfigHandler::processConnectResponse(JmyMsgInfo* info)
{
	return 0;
}

int ConnConfigHandler::processNewLoginNotify(JmyMsgInfo* info)
{
	return 0;
}

int ConnConfigHandler::onConnect(JmyEventInfo* info)
{
	return 0;
}

int ConnConfigHandler::onDisconnect(JmyEventInfo* info)
{
	return 0;
}

int ConnConfigHandler::onTick(JmyEventInfo* info)
{
	return 0;
}

int ConnConfigHandler::onTimer(JmyEventInfo* info)
{
	return 0;
}
