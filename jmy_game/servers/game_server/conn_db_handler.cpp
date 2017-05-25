#include "conn_db_handler.h"
#include "../libjmy/jmy_tcp_connection.h"
#include "../common/util.h"
#include "../../proto/src/server.pb.h"
#include "config_loader.h"
#include "global_data.h"
#include "player.h"

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

int ConnDBHandler::processGetRoleResponse(JmyMsgInfo* info)
{
	MsgDS2GS_GetRoleResponse response;
	if (!response.ParseFromArray(info->data, info->len)) {
		LogError("parse MsgGS2GT_GetRoleResponse failed");
		return -1;
	}

	MsgGS2GT_GetRoleResponse get_resp;
	get_resp.set_account(response.account());
	get_resp.mutable_role_data()->set_sex(response.role_data().sex());
	get_resp.mutable_role_data()->set_race(response.role_data().race());
	get_resp.mutable_role_data()->set_role_id(response.role_data().role_id());
	get_resp.mutable_role_data()->set_nick_name(response.role_data().nick_name());
	if (!get_resp.SerializeToArray(tmp_, sizeof(tmp_))) {
		LogError("serialize MsgGS2GT_GetRoleResponse failed");
		return -1;
	}

	if (SEND_GATE_MSG(MSGID_GS2GT_GET_ROLE_RESPONSE, tmp_, get_resp.ByteSize()) < 0) {
		LogError("send MsgGS2GT_GetRoleResponse failed");
		return -1;
	}

	LogInfo("send MsgGS2GT_GetRoleResponse");

	return info->len;
}

int ConnDBHandler::processCreateRoleResponse(JmyMsgInfo* info)
{
	MsgDS2GS_CreateRoleResponse response;
	if (!response.ParseFromArray(info->data, info->len)) {
		LogError("parse MsgDS2GS_CreateRoleResponse failed");
		return -1;
	}

	MsgGS2GT_CreateRoleResponse create_resp;
	create_resp.set_account(response.account());
	create_resp.mutable_role_data()->set_sex(response.role_data().sex());
	create_resp.mutable_role_data()->set_race(response.role_data().race());
	create_resp.mutable_role_data()->set_nick_name(response.role_data().nick_name());
	if (!create_resp.SerializeToArray(tmp_, sizeof(tmp_))) {
		LogError("serialize MsgGS2GT_CreateRoleResponse failed");
		return -1;
	}

	if (SEND_GATE_MSG(MSGID_GS2GT_CREATE_ROLE_RESPONSE, tmp_, create_resp.ByteSize()) < 0) {
		LogError("send MsgGS2GT_CreateRoleResponse failed");
		return -1;
	}

	LogInfo("send MsgGS2GT_CreateRoleResponse");
	return info->len;
}

int ConnDBHandler::processEnterGameResponse(JmyMsgInfo* info)
{
#if 0
	int user_id = PLAYER_MGR->getUserIdByAccount(response.account());
	if (user_id <= 0) {
		LogError("get user id by account(%s) failed", response.account().c_str());
		return -1;
	}
	Player* p = PLAYER_MGR->malloc(user_id, response.user_data().uid());
	if (!p) {
		LogError("malloc new Player by pair(user_id:%d, unique_id:%llu) failed", user_id, response.user_data().uid());
		return -1;
	}
	
	MsgGS2GT_EnterGameResponse rsp_to_gt;
	rsp_to_gt.set_account(response.account());
	if (!rsp_to_gt.SerializeToArray(tmp_, sizeof(tmp_))) {
		LogError("serialize MsgGS2GT_EnterGameResponse failed");
		return -1;
	}
	if (GLOBAL_DATA->sendGate(user_id, MSGID_GS2GT_ENTER_GAME_RESPONSE, tmp_, rsp_to_gt.ByteSize()) < 0) {
		LogError("send MsgGS2GT_EnterGameResponse failed");
		return -1;
	}
#endif
	LogInfo("processEnterGameResponse");
	return info->len;
}

int ConnDBHandler::processDefault(JmyMsgInfo* info)
{
	return info->len;
}
