#include "game_handler.h"
#include "../common/util.h"
#include "../../proto/src/msgid.pb.h"
#include "../../proto/src/server.pb.h"
#include "client_handler.h"
#include "config_loader.h"
#include "global_data.h"
#include "client_manager.h"

char GameHandler::tmp_[JMY_MAX_MSG_SIZE];
char GameHandler::session_buf_[RECONN_SESSION_CODE_BUF_LENGTH+1];

int GameHandler::onConnect(JmyEventInfo* info)
{
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) {
		LogError("get connection failed by info");
		return -1;
	}
	LogInfo("game server onConnect, conn_id(%d)", info->conn_id);
	return 0;
}

int GameHandler::onDisconnect(JmyEventInfo* info)
{
	if (!GLOBAL_DATA->removeGameAgentByConnId(info->conn_id)) {
		LogError("cant remove game agent by conn_id(%d)", info->conn_id);
		return -1;
	}
	LogInfo("game server onDisconnect, conn_id(%d)", info->conn_id);
	return 0;
}

int GameHandler::onTick(JmyEventInfo* info)
{
	(void)info;
	return 0;
}

int GameHandler::processConnectGateRequest(JmyMsgInfo* info)
{
	MsgGS2GT_ConnectGateRequest request;
	if (!request.ParseFromArray(info->data, info->len)) {
		LogError("parse message MsgGS2GT_ConnectGateRequest failed");
		return -1;
	}
	int game_id = request.game_id();

	GameAgent* agent = GLOBAL_DATA->newGameAgent(game_id, (JmyTcpConnectionMgr*)info->param, info->conn_id);
	if (!agent) {
		LogError("create new agent with game_id(%d), conn_id(%d) failed", game_id, info->conn_id);
		return -1;
	}

	// return message to game server
	MsgGT2GS_ConnectGateResponse response;
	response.set_max_user_count(CONFIG_FILE.max_conn);
	response.set_start_user_id(CLIENT_MANAGER->getClientStartId());
	if (!response.SerializeToArray(tmp_, sizeof(tmp_))) {
		LogError("serialize msg MsgGT2GS_ConnectGateResponse failed");
		return -1;
	}
	if (agent->sendMsg(MSGID_GT2GS_CONNECT_GATE_RESPONSE, tmp_, response.ByteSize()) < 0) {
		LogError("send msg MsgGT2GS_ConnectGateResponse failed");
		return -1;
	}
	
	LogInfo("game_server(id: %d) connected", game_id);
	return info->len;
}

int GameHandler::processGetRoleResponse(JmyMsgInfo* info)
{
	MsgGS2GT_GetRoleResponse response;
	if (!response.ParseFromArray(info->data, info->len)) {
		LogError("parse MsgGS2GT_GetRoleResponse failed");
		return -1;
	}

	ClientInfo* ci = CLIENT_MANAGER->getClientInfoByAccount(response.account());
	if (!ci) {
		LogError("get ClientInfo by account %s failed", response.account().c_str());
		return -1;
	}

	uint64_t role_id = response.role_data().role_id();
	if (role_id == 0) {
		MsgError error;
		error.set_error_code(PROTO_ERROR_GET_ROLE_NONE);
		if (!error.SerializeToArray(tmp_, sizeof(tmp_))) {
			LogError("serialize MsgError failed");
			return -1;
		}
		if (ci->send(MSGID_ERROR, tmp_, error.ByteSize()) < 0) {
			LogError("send MsgError failed");
			return -1;
		}
		static int count = 0;
		LogInfo("get account(%s) not found, send error: PROTO_ERROR_GET_ROLE_NONE, count(%d)", response.account().c_str(), ++count);
	} else {
		MsgS2C_GetRoleResponse get_resp;
		char* reconn_session = get_session_code(session_buf_, RECONN_SESSION_CODE_BUF_LENGTH);
		ci->reconn_session = reconn_session;
		get_resp.set_reconnect_session(reconn_session);
		get_resp.mutable_role_data()->set_sex(response.role_data().sex());
		get_resp.mutable_role_data()->set_race(response.role_data().race());
		get_resp.mutable_role_data()->set_role_id(response.role_data().role_id());
		get_resp.mutable_role_data()->set_nick_name(response.role_data().nick_name());
		if (!response.SerializeToArray(tmp_, sizeof(tmp_))) {
			LogError("serialize MsgS2C_GetRoleResponse failed");
			return -1;
		}
		if (ci->send(MSGID_S2C_GET_ROLE_RESPONSE, tmp_, response.ByteSize()) < 0) {
			LogError("send MsgS2C_GetRoleResponse failed");
			return -1;
		}
		ci->add_role_id(role_id);
		ci->curr_role_id = role_id;
		LogInfo("get role: %llu(account:%s) response", role_id, response.account().c_str());
	}

	return info->len;
}

int GameHandler::processCreateRoleResponse(JmyMsgInfo* info)
{
	MsgGS2GT_CreateRoleResponse response;
	if (!response.ParseFromArray(info->data, info->len)) {
		LogError("parse MsgGS2GT_CreateRoleResponse failed");
		return -1;
	}

	ClientInfo* ci = CLIENT_MANAGER->getClientInfoByAccount(response.account());
	if (!ci) {
		LogError("get ClientInfo by account %s failed", response.account().c_str());
		return -1;
	}

	uint64_t role_id = response.role_data().role_id();
	ci->add_role_id(role_id);
	ci->curr_role_id = role_id;

	MsgS2C_CreateRoleResponse create_resp;
	create_resp.mutable_role_data()->set_sex(response.role_data().sex());
	create_resp.mutable_role_data()->set_race(response.role_data().race());
	create_resp.mutable_role_data()->set_role_id(response.role_data().role_id());
	create_resp.mutable_role_data()->set_nick_name(response.role_data().nick_name());
	if (!create_resp.SerializeToArray(tmp_, sizeof(tmp_))) {
		LogError("serialize MsgS2C_CreateRoleResponse failed");
		return -1;
	}
	if (ci->send(MSGID_S2C_CREATE_ROLE_RESPONSE, tmp_, create_resp.ByteSize()) < 0) {
		LogError("send MsgS2C_CreateRoleResponse failed");
		return -1;
	}

	LogInfo("create role(account:%s, role_id:%llu) response", response.account().c_str(), role_id);
	return info->len;
}

int GameHandler::processDefault(JmyMsgInfo* info)
{
	switch (info->msg_id) {
	default:
		{
			JmyTcpConnection* conn = get_connection(info);
			if (!conn) {
				LogError("get connection failed by info");
				return -1;
			}
			if (!info->user_id) return -1;
			ClientInfo* ci = CLIENT_MANAGER->getClientInfo(info->user_id);
			if (!ci) {
				LogError("get ClientInfo by user_id(%d) failed", info->user_id);
				return -1;
			}
			if (ci->send(info->msg_id, info->data, info->len) < 0) {
				LogError("send message: %d with default failed", info->msg_id);
				return -1;
			}
			//LogInfo("process default msg(%d)", info->msg_id);
		}
		break;
	}
	return info->len;
}
