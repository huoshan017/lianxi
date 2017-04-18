#include "conn_db_handler.h"
#include "../libjmy/jmy_tcp_connection.h"
#include "../common/util.h"
#include "../../proto/src/server.pb.h"
#include "config_loader.h"

char ConnDBHandler::tmp_[JMY_MAX_MSG_SIZE];

int ConnDBHandler::onConnect(JmyEventInfo* info)
{
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) {
		LogError("get connection failed in db onConnect");
		return -1;
	}

	MsgGS2DS_ConnectDBRequest request;
	request.set_game_id(SERVER_CONFIG.id);
	if (!request.SerializeToArray(tmp_, sizeof(tmp_))) {
		LogError("serialize msg MsgGS2DS_ConnectDBRequest failed");
		return -1;
	}
	if (conn->send(MSGID_GS2DS_CONNECT_DB_REQUEST, tmp_, request.ByteSize()) < 0) {
		LogError("send msg MsgGS2DS_ConnectDBRequest failed");
		return -1;
	}
	LogInfo("db onConnect");
	return 0;
}

int ConnDBHandler::onDisconnect(JmyEventInfo* info)
{
	(void)info;
	LogInfo("db onDisconnect");
	return 0;
}

int ConnDBHandler::onTick(JmyEventInfo* info)
{
	(void)info;
	return 0;
}

int ConnDBHandler::processConnectDBResponse(JmyMsgInfo* info)
{
	return info->len;
}

int ConnDBHandler::processRequireUserDataResponse(JmyMsgInfo* info)
{
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) {
		LogError("get connection failed in processRequireUserDataResponse");
		return -1;
	}

	return info->len;
}
