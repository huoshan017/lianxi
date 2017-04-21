#include "conn_db_handler.h"
#include "../libjmy/jmy_tcp_connection.h"
#include "../common/util.h"
#include "../../proto/src/server.pb.h"
#include "config_loader.h"
#include "global_data.h"

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
	GLOBAL_DATA->setDBConn(nullptr);
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
	MsgDS2GS_ConnectDBResponse response;
	if (!response.ParseFromArray(info->data, info->len)) {
		LogError("parse MsgDS2GS_ConnectDBResponse failed");
		return -1;
	}

	JmyTcpConnection* conn = get_connection(info);
	if (!conn) {
		LogError("get connection failed");
		return -1;
	}

	GLOBAL_DATA->setDBConn(conn);

	LogInfo("processConnectDBResponse: connect db_server");
	return info->len;
}

int ConnDBHandler::processRequireUserDataResponse(JmyMsgInfo* info)
{
	MsgDS2GS_RequireUserDataResponse response;
	if (!response.ParseFromArray(info->data, info->len)) {
		LogError("parse MsgGS2GT_EnterGameResponse failed");
		return -1;
	}
	
	MsgGS2GT_EnterGameResponse rsp_to_gt;
	if (!rsp_to_gt.SerializeToArray(tmp_, sizeof(tmp_))) {
		LogError("serialize MsgGS2GT_EnterGameResponse failed");
		return -1;
	}
	if (GLOBAL_DATA->sendGate(info->user_id, MSGID_GS2GT_ENTER_GAME_RESPONSE, tmp_, rsp_to_gt.ByteSize()) < 0) {
		LogError("send MsgGS2GT_EnterGameResponse failed");
		return -1;
	}
	LogInfo("processRequireUserDataResponse: ");
	return info->len;
}

int ConnDBHandler::processDefault(JmyMsgInfo* info)
{
	return info->len;
}
