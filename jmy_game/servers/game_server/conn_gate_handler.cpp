#include "conn_gate_handler.h"
#include "../libjmy/jmy_datatype.h"

char ConnGateHandler::tmp_[JMY_MAX_MSG_SIZE];

int ConnGateHandler::onConnect(JmyEventInfo* info)
{
	return 0;
}

int ConnGateHandler::onDisconnect(JmyEventInfo* info)
{
	return 0;
}

int ConnGateHandler::onTick(JmyEventInfo* info)
{
	return 0;
}

int ConnGateHandler::onTimer(JmyEventInfo* info)
{
	return 0;
}

int ConnGateHandler::processEnterGame(JmyMsgInfo* info)
{
	return info->len;
}

int ConnGateHandler::processLeaveGame(JmyMsgInfo* info)
{
	return info->len;
}

int ConnGateHandler::processDefault(JmyMsgInfo* info)
{
	return info->len;
}
