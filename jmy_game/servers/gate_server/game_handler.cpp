#include "game_handler.h"

char GameHandler::tmp_[MAX_SEND_BUFFER_SIZE];

int GameHandler::processConnectRequest(JmyMsgInfo* info)
{
	return 0;
}

int GameHandler::onConnect(JmyEventInfo* info)
{
	return 0;
}

int GameHandler::onDisconnect(JmyEventInfo* info)
{
	return 0;
}

int GameHandler::onTick(JmyEventInfo* info)
{
	(void)info;
	return 0;
}

int GameHandler::onTimer(JmyEventInfo* info)
{
	(void)info;
	return 0;
}
