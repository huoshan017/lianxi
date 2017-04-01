#include "conn_event_handler.h"
#include "../libjmy/jmy_datatype.h"
#include "../libjmy/jmy_tcp_connection.h"
#include "../common/util.h"

char ConnEventHandler::tmp_[MAX_SEND_BUFFER_SIZE];

int ConnEventHandler::onConnect(JmyEventInfo* info)
{
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) return -1;
	return 0;
}

int ConnEventHandler::onDisconnect(JmyEventInfo* info)
{
	(void)info;
	return 0;
}

int ConnEventHandler::onTick(JmyEventInfo* info)
{
	(void)info;
	return 0;
}

int ConnEventHandler::onTimer(JmyEventInfo* info)
{
	(void)info;
	return 0;
}
