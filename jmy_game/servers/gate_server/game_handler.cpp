#include "game_handler.h"
#include "../common/util.h"
#include "../../proto/src/server.pb.h"
#include "client_handler.h"

char GameHandler::tmp_[JMY_MAX_MSG_SIZE];
GameAgentManager GameHandler::game_mgr_;
int GameHandler::the_game_id_ = 0;

int GameHandler::onConnect(JmyEventInfo* info)
{
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) return -1;
	ServerLogInfo("game server onconnect, conn_id(%d)", info->conn_id);
	return 0;
}

int GameHandler::onDisconnect(JmyEventInfo* info)
{
	GameAgent* agent = game_mgr_.getAgentByConnId(info->conn_id);
	if (!agent) {
		return 0;
	}

	if (agent->getId() != the_game_id_) {
		ServerLogError("game agent id not equal to the game id ondisconnect");
		return -1;
	}

	game_mgr_.deleteAgent(the_game_id_);
	the_game_id_ = 0;

	ServerLogInfo("game server ondisconnect, conn_id(%d)", info->conn_id);
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
	GameAgent* agent = game_mgr_.getAgentByConnId(info->session_id);
	if (agent) {
		ServerLogWarn("already exist game_server connection(conn_id: %d)", info->session_id);
		return 0;
	}

	if (the_game_id_ > 0) {
		ServerLogError("to game agent(%d) connection is normal, can not allow to other game agent connect", the_game_id_);
		return -1;
	}

	MsgGS2GT_ConnectGateRequest request;
	int game_id = request.game_id();
	agent = game_mgr_.newAgent(game_id, (JmyTcpConnectionMgr*)info->param, info->session_id);
	if (!agent) {
		return 0;
	}

	the_game_id_ = game_id;
	
	ServerLogInfo("game_server(id: %d) connected", game_id);
	return info->len;
}

int GameHandler::processEnterGameResponse(JmyMsgInfo* info)
{
	ClientInfo* client_info = GET_CLIENT_INFO(info->receiver_id);
	if (!client_info) {
		SEND_CLIENT_ERROR(client_info->conn, PROTO_ERROR_ENTER_GAME_INVALID_ACCOUNT);
		ServerLogError("cant get ClientInfo by id(%d)", info->receiver_id);
		return -1;
	}
	if (ClientHandler::sendEnterGameResponse2Client(info->receiver_id) < 0) {
		ServerLogError("send enter game response to client failed");
		return -1;
	}

	MsgGS2GT_EnterGameResponse response;
	if (!response.ParseFromArray(info->data, info->len)) {
		ServerLogError("parse MsgGS2GT_EnterGameResponse failed");
		return -1;
	}
	ServerLogInfo("player %s enter game success", response.account().c_str());
	return info->len;
}

int GameHandler::processLeaveGameResponse(JmyMsgInfo* info)
{
	ClientInfo* client_info = GET_CLIENT_INFO(info->receiver_id);
	if (!client_info) {
		ServerLogError("cant get ClientInfo by id(%d)", info->receiver_id);
		return -1;
	}
	client_info->close();
	ServerLogInfo("player %s leave game");
	return info->len;
}

int GameHandler::processDefault(JmyMsgInfo* info)
{
	return info->len;
}

int GameHandler::sendMsg(int user_id, int msg_id, const char* data, unsigned short len)
{
	GameAgent* agent = game_mgr_.getAgent(the_game_id_);
	if (!agent) {
		ServerLogError("cant found game agent(%d) connection", the_game_id_);
		return -1;
	}

	if (agent->sendMsg(user_id, msg_id, data, len) < 0) {
		ServerLogError("send user(%d) message(%d) failed", user_id, msg_id);
		return -1;
	}
	return len;
}
