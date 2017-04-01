#include "client_event_handler.h"
#include "../libjmy/jmy_tcp_connection.h"
#include "../common/util.h"

int ClientEventHandler::onConnect(JmyEventInfo* info)
{
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) return -1;
	return 0;
}

int ClientEventHandler::onDisconnect(JmyEventInfo* info)
{
	return 0;
}

int ClientEventHandler::onTick(JmyEventInfo* info)
{
	return 0;
}

int ClientEventHandler::onTimer(JmyEventInfo* info)
{
	return 0;
}
