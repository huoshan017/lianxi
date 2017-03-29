#include "gate_msg_handler.h"
#include "../common/util.h"

int GateMsgHandler::processConnect(JmyMsgInfo* info)
{
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) {
		return -1;
	}
	return 0;
}
