#include "game_handler.h"
#include "../common/util.h"
#include "../../proto/src/server.pb.h"

char GameHandler::tmp_[JMY_MAX_MSG_SIZE];
GameAgentManager GameHandler::game_mgr_;

int GameHandler::onConnect(JmyEventInfo* info)
{
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) return -1;
	ServerLogInfo("game server onconnect, conn_id(%d)", info->conn_id);
	return 0;
}

int GameHandler::onDisconnect(JmyEventInfo* info)
{
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

	MsgGS2GT_ConnectGateRequest request;
	int game_id = request.game_id();
	agent = game_mgr_.newAgent(game_id, (JmyTcpConnectionMgr*)info->param, info->session_id);
	if (!agent) {
		return 0;
	}
	
	ServerLogInfo("game_server(id: %d) connected", game_id);
	return info->len;
}
