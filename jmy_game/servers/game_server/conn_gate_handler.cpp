#include "conn_gate_handler.h"
#include "../libjmy/jmy_datatype.h"
#include "../libjmy/jmy_tcp_connection.h"
#include "../common/util.h"
#include "../../proto/src/msgid.pb.h"
#include "../../proto/src/account.pb.h"
#include "../../proto/src/test.pb.h"
#include "../../proto/src/server.pb.h"
#include "config_loader.h"
#include "player.h"
#include "global_data.h"
#include "gm.h"

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

	static int get_count = 0;
	LogInfo("process get role(account:%s) request, get_count(%d)", request.account().c_str(), ++get_count);
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

	static int create_count = 0;
	LogInfo("processCreateRole: create role for account(%s), create_count(%d)", request.account().c_str(), ++create_count);
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
		LogError("serialize MsgS2C_EnterGameResponse failed");
		return -1;
	}

	if (SEND_GATE_USER_MSG(user_id, MSGID_S2C_ENTER_GAME_RESPONSE, tmp_, response.ByteSize()) < 0) {
		LogError("send MsgS2C_EnterGameResponse failed");
		return -1;
	}

	MsgS2C_EnterGameCompleteNotify complete_notify;
	if (!response.SerializeToArray(tmp_, sizeof(tmp_))) {
		LogError("serialize MsgS2C_EnterGameCompleteNotify failed");
		return -1;
	}

	if (SEND_GATE_USER_MSG(user_id, MSGID_S2C_ENTER_GAME_COMPLETE_NOTIFY, tmp_, response.ByteSize()) < 0) {
		LogError("serialize MsgS2C_EnterGameCompleteNotify failed");
		return -1;
	}
	
	static int enter_count = 0;
	LogInfo("processEnterGame: role_id(%llu), user_id(%d), enter_count(%d)", request.role_id(), user_id, ++enter_count);
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

	PLAYER_MGR->free(request.role_id());
	
	LogInfo("processLeaveGame: role_id(%llu), user_id(%d)", request.role_id(), info->user_id);
	return info->len;
}

int ConnGateHandler::processDefault(JmyMsgInfo* info)
{
	switch (info->msg_id) {
	case MSGID_C2S_CHAT_REQUEST:
		return processChat(info);
	case MSGID_C2S_SET_ROLE_DATA_REQUEST:
		return processSetRoleData(info);
	default:
		LogWarn("not handled msg_id(%d)", info->msg_id);
		break;
	}
	return info->len;
}

int ConnGateHandler::processChat(JmyMsgInfo* info)
{
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) {
		LogError("get connection failed");
		return -1;
	}

	MsgC2S_ChatRequest request;
	if (!request.ParseFromArray(info->data, info->len)) {
		LogError("parse MsgC2S_ChatRequest failed");
		return -1;
	}
	const std::string& content = request.content();
	std::vector<std::string> results;
	if (GM_MGR->parse_command(content, results)) {
		Player* p = PLAYER_MGR->get(info->user_id);
		if (!p) return 0;
		if (!GM_MGR->execute_command(p->role_id, results))
			return 0;
		LogInfo("executed gm command");
		return info->len;
	}

	// response
	MsgS2C_ChatResponse response;
	response.set_chat_type(request.chat_type());
	response.set_channel(request.channel());
	response.set_content(request.content());
	response.set_role_id(request.role_id());
	if (!response.SerializeToArray(tmp_, sizeof(tmp_))) {
		LogError("serialize MsgS2C_ChatResponse failed");
		return -1;
	}
	if (conn->send(info->user_id, MSGID_S2C_CHAT_RESPONSE, tmp_, response.ByteSize()) < 0) {
		LogError("send MsgS2C_ChatResponse failed");
		return -1;
	}
	LogInfo("chat response");

	Player* p = PLAYER_MGR->getByRoleId(request.role_id());
	if (!p) {
		LogWarn("player by role_id(%llu) not found", request.role_id());
		return 0;
	}
	// notify
	MsgS2C_ChatNotify notify;
	notify.set_chat_type(request.chat_type());
	notify.set_channel(request.channel());
	if (!notify.SerializeToArray(tmp_, sizeof(tmp_))) {
		LogError("serialize MsgS2C_ChatNotify failed");
		return -1;
	}
	if (p->send_gate(MSGID_S2C_CHAT_NOTIFY, tmp_, notify.ByteSize()) < 0) {
		LogError("send MsgS2C_ChatNotify failed");
		return -1;
	}
	LogInfo("chat notify to role(%llu) failed", request.role_id());

	return info->len;
}

int ConnGateHandler::processSetRoleData(JmyMsgInfo* info)
{
	MsgC2S_SetRoleDataRequest request;
	if (!request.ParseFromArray(info->data, info->len)) {
		LogError("parse MsgC2S_SetRoleDataRequest failed");
		return -1;
	}

	MsgGS2DS_SetRoleDataRequest set_req;
	uint64_t role_id = PLAYER_MGR->getRoleIdByUserId(info->user_id);
	set_req.set_role_id(role_id);
	MsgBaseRoleData* role_data = set_req.mutable_role_data();
	const MsgBaseRoleData& const_role_data = request.role_data();
	role_data->set_hp(const_role_data.hp());
	role_data->set_sex(const_role_data.sex());
	role_data->set_race(const_role_data.race());
	role_data->set_level(const_role_data.level());
	if (!set_req.SerializeToArray(tmp_, sizeof(tmp_))) {
		LogError("serialize MsgGS2DS_SetRoleDataRequest failed");
		return -1;
	}
	if (SEND_DB_MSG(MSGID_GS2DS_SET_ROLE_DATA_REQUEST, tmp_, set_req.ByteSize()) < 0) {
		LogError("send MsgGS2DS_SetRoleDataRequest failed");
		return -1;
	}
	LogInfo("send MsgGS2DS_SetRoleDataRequest");
	return info->len;
}
