#include "game_handler.h"
#include "../common/util.h"
#include "../../proto/src/server.pb.h"
#include "../../proto/src/common.pb.h"
#include "client_handler.h"
#include "config_loader.h"
#include "global_data.h"
#include "client_manager.h"

char GameHandler::tmp_[JMY_MAX_MSG_SIZE];
char GameHandler::session_buf_[RECONN_SESSION_CODE_BUF_LENGTH+1];

int GameHandler::onConnect(JmyEventInfo* info)
{
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) return -1;
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

	MsgS2C_GetRoleResponse get_resp;
	char* reconn_session = get_session_code(session_buf_, RECONN_SESSION_CODE_BUF_LENGTH);
	ci->reconn_session = reconn_session;
	get_resp.set_reconnect_session(reconn_session);
	get_resp.set_max_role_count(1);
	*get_resp.mutable_role_list() = *response.mutable_role_list();
	if (!response.SerializeToArray(tmp_, sizeof(tmp_))) {
		LogError("serialize MsgS2C_VerifyResponse failed");
		return -1;
	}

	if (ci->send(MSGID_S2C_GET_ROLE_RESPONSE, tmp_, response.ByteSize()) < 0) {
		LogError("send MsgS2C_VerifyResponse failed");
		return -1;
	}

	return info->len;
}

int GameHandler::processCreateRoleResponse(JmyMsgInfo* info)
{
	return info->len;
}

#if 0
int GameHandler::processDeleteRoleResponse(JmyMsgInfo* info)
{
	return info->len;
}
#endif

int GameHandler::processEnterGameResponse(JmyMsgInfo* info)
{
	MsgGS2GT_EnterGameResponse response;
	if (!response.ParseFromArray(info->data, info->len)) {
		LogError("parse MsgGS2GT_EnterGameResponse failed");
		return -1;
	}

	ClientInfo* client_info = CLIENT_MANAGER->getClientInfo(info->user_id);
	if (!client_info) {
		//send_error(client_info->conn, PROTO_ERROR_ENTER_GAME_INVALID_ACCOUNT);
		LogError("cant get ClientInfo by id(%d)", info->user_id);
		return -1;
	}

	MsgS2C_EnterGameResponse rsp_to_clt;
	rsp_to_clt.set_role_id(client_info->curr_uid);
	//rsp_to_clt.set_reconnect_session(client_info->reconn_session.c_str());
	if (!response.SerializeToArray(tmp_, sizeof(tmp_))) {
		LogError("serialize MsgS2C_EnterGameResponse failed");
		return -1;
	}
	
	if (client_info->send(MSGID_S2C_ENTER_GAME_RESPONSE, tmp_, response.ByteSize()) < 0) {
		LogError("send message MsgS2C_EnterGameResponse failed to client(%d) failed", info->user_id);
		return -1;
	}

	LogInfo("user_id(%llu) enter game success", client_info->curr_uid);
	return info->len;
}

int GameHandler::processLeaveGameResponse(JmyMsgInfo* info)
{
	ClientInfo* client_info = CLIENT_MANAGER->getClientInfo(info->user_id);
	if (!client_info) {
		LogError("cant get ClientInfo by id(%d)", info->user_id);
		return -1;
	}
	client_info->close();
	LogInfo("player %s leave game");
	return info->len;
}

int GameHandler::processDefault(JmyMsgInfo* info)
{
	return info->len;
}
