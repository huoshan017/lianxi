#include "game_handler.h"
#include "../common/util.h"
#include "../../proto/src/server.pb.h"
#include "client_handler.h"
#include "config_loader.h"
#include "global_data.h"

char GameHandler::tmp_[JMY_MAX_MSG_SIZE];

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

int GameHandler::onTimer(JmyEventInfo* info)
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
	response.set_start_user_id(ClientHandler::getClientStartId());
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

int GameHandler::processEnterGameResponse(JmyMsgInfo* info)
{
	ClientInfo* client_info = GET_CLIENT_INFO(info->user_id);
	if (!client_info) {
		SEND_CLIENT_ERROR(client_info->conn, PROTO_ERROR_ENTER_GAME_INVALID_ACCOUNT);
		LogError("cant get ClientInfo by id(%d)", info->user_id);
		return -1;
	}
	if (ClientHandler::sendEnterGameResponse2Client(info->user_id) < 0) {
		LogError("send enter game response to client failed");
		return -1;
	}

	MsgGS2GT_EnterGameResponse response;
	if (!response.ParseFromArray(info->data, info->len)) {
		LogError("parse MsgGS2GT_EnterGameResponse failed");
		return -1;
	}
	LogInfo("player %s enter game success", response.account().c_str());
	return info->len;
}

int GameHandler::processLeaveGameResponse(JmyMsgInfo* info)
{
	ClientInfo* client_info = GET_CLIENT_INFO(info->user_id);
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
