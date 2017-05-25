#include "conn_gate_handler.h"
#include "../libjmy/jmy_datatype.h"
#include "../libjmy/jmy_tcp_connection.h"
#include "../common/util.h"
#include "../../proto/src/common.pb.h"
#include "../../proto/src/server.pb.h"
#include "config_loader.h"
#include "player.h"
#include "global_data.h"

char ConnGateHandler::tmp_[JMY_MAX_MSG_SIZE];

int ConnGateHandler::onConnect(JmyEventInfo* info)
{
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) return -1;
	MsgGS2GT_ConnectGateRequest request;
	request.set_game_id(SERVER_CONFIG.id);
	if (!request.SerializeToArray(tmp_, sizeof(tmp_))) {
		LogError("serialize MsgGS2GT_ConnectGateRequest failed");
		return -1;
	}
	if (conn->send(MSGID_GS2GT_CONNECT_GATE_REQUEST, tmp_, request.ByteSize()) < 0) {
		LogError("send message MsgGS2GT_ConnectGateRequest failed");
		return -1;
	}
	LogInfo("send message MsgGS2GT_ConnectGateRequest to gate server");
	return 0;
}

int ConnGateHandler::onDisconnect(JmyEventInfo* info)
{
	(void)info;
	GLOBAL_DATA->setGateConn(nullptr);
	LogInfo("ondisconnect");
	return 0;
}

int ConnGateHandler::onTick(JmyEventInfo* info)
{
	(void)info;
	return 0;
}

int ConnGateHandler::processConnectGateResponse(JmyMsgInfo* info)
{
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) {
		LogError("cant get connection");
		return -1;
	}

	MsgGT2GS_ConnectGateResponse response;
	if (!response.ParseFromArray(info->data, info->len)) {
		LogError("parse MsgGT2GS_ConnectGateResponse failed");
		return -1;
	}

	if (!PLAYER_MGR->init(response.start_user_id(), response.max_user_count())) {
		LogError("PlayerManager init with start_user_id(%d) max_user_count(%d) failed",
				response.start_user_id(), response.max_user_count());
		return -1;
	}

	GLOBAL_DATA->setGateConn(conn);

	LogInfo("processConnectGateResponse: start_user_id(%d), max_user_count(%d)",
			response.start_user_id(), response.max_user_count());
	
	return info->len;
}

int ConnGateHandler::processGetRole(JmyMsgInfo* info)
{
	MsgGT2GS_GetRoleRequest request;
	if (!request.ParseFromArray(info->data, info->len)) {
		LogError("parse MsgGT2GS_GetRoleRequest failed");
		return -1;
	}

	MsgGS2DS_GetRoleRequest get_req;
	get_req.set_account(request.account());
	if (!get_req.SerializeToArray(tmp_, sizeof(tmp_))) {
		LogError("serialize MsgGS2DS_GetRoleRequest failed");
		return -1;
	}
	if (SEND_DB_MSG(MSGID_GS2DS_GET_ROLE_REQUEST, tmp_, get_req.ByteSize()) < 0) {
		LogError("send MsgGS2DS_GetRoleRequest failed");
		return -1;
	}
	LogInfo("process get role(account:%s) request", request.account().c_str());
	return info->len;
}

int ConnGateHandler::processCreateRole(JmyMsgInfo* info)
{
	MsgGT2GS_CreateRoleRequest request;
	if (!request.ParseFromArray(info->data, info->len)) {
		LogError("parse MsgGT2GS_CreateRoleRequest failed");
		return -1;
	}

	MsgGS2DS_CreateRoleRequest create_req;
	create_req.set_account(request.account());
	create_req.set_nick_name(request.nick_name());
	create_req.set_sex(request.sex());
	create_req.set_race(request.race());
	if (!create_req.SerializeToArray(tmp_, sizeof(tmp_))) {
		LogError("serialize MsgGS2DS_CreateRoleRequest failed");
		return -1;
	}
	if (SEND_DB_MSG(MSGID_GS2DS_CREATE_ROLE_REQUEST, tmp_, create_req.ByteSize()) < 0) {
		LogError("send MsgGS2DS_CreateRoleRequest failed");
		return -1;
	}
	LogInfo("processCreateRole: create role for account(%s)", request.account().c_str());
	return info->len;
}

int ConnGateHandler::processEnterGame(JmyMsgInfo* info)
{
	MsgGT2GS_EnterGameRequest request;
	if (!request.ParseFromArray(info->data, info->len)) {
		LogError("parse MsgGT2GS_EnterGameRequest failed");
		return -1;
	}

	int user_id = info->user_id;
	if (user_id <= 0) {
		LogError("user_id %d invalid", user_id);
		return -1;
	}

	Player* p = PLAYER_MGR->get(user_id);
	if (!p) {
		p = PLAYER_MGR->malloc(user_id, request.role_id());
	}

	MsgS2C_EnterGameResponse response;
	if (!response.SerializeToArray(tmp_, sizeof(tmp_))) {
		LogError("serialize MsgGS2GT_EnterGameResponse failed");
		return -1;
	}

	if (SEND_GATE_USER_MSG(user_id, MSGID_S2C_ENTER_GAME_RESPONSE, tmp_, response.ByteSize()) < 0) {
		LogError("send MsgGS2GT_EnterGameResponse failed");
		return -1;
	}
	
	LogInfo("processEnterGame: user_id(%d)", user_id);
	return info->len;
}

int ConnGateHandler::processLeaveGame(JmyMsgInfo* info)
{
	MsgGT2GS_LeaveGameRequest request;
	if (!request.ParseFromArray(info->data, info->len)) {
		LogError("parse MsgGT2GS_LeaveGameRequest failed");
		return -1;
	}

	Player* p = PLAYER_MGR->get(info->user_id);
	if (!p) {
		LogError("cant get Player by user_id(%d)", info->user_id);
		return -1;
	}
	
	LogInfo("processLeaveGame: user_id(%d)", info->user_id);
	return info->len;
}

int ConnGateHandler::processDefault(JmyMsgInfo* info)
{
	switch (info->msg_id) {
	case MSGID_C2S_ENTER_GAME_REQUEST:
		return processEnterGame(info);
	case MSGID_C2S_LEAVE_GAME_REQUEST:
		return processLeaveGame(info);	
	default:
		LogWarn("not handled msg_id(%d)", info->msg_id);
		break;
	}
	return info->len;
}
