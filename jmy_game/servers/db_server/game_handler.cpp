#include "game_handler.h"
#include "../common/util.h"
#include "../../proto/src/server.pb.h"
#include "config_loader.h"

GameAgentManager GameHandler::game_mgr_;
UserDataManager GameHandler::user_mgr_;
MysqlConnectorPool GameHandler::conn_pool_;
char GameHandler::tmp_[JMY_MAX_MSG_SIZE];
std::set<std::string> GameHandler::accounts_set_;

bool GameHandler::init()
{
	// init mysql connector pool
	MysqlConnPoolConfig conn_pool_config(
			const_cast<char*>(SERVER_CONFIG.mysql_host.c_str()),
			const_cast<char*>(SERVER_CONFIG.mysql_user.c_str()),
			const_cast<char*>(SERVER_CONFIG.mysql_password.c_str()),
			const_cast<char*>(SERVER_CONFIG.mysql_dbname.c_str()));
	if (!conn_pool_.init(conn_pool_config)) {
		ServerLogError("init mysql connector pool failed");
		return false;
	}
	return true;
}

void GameHandler::clear()
{
	conn_pool_.close();
}

int GameHandler::onConnect(JmyEventInfo* info)
{
	ServerLogInfo("onconnection conn_id(%d)", info->conn_id);
	return 0;
}

int GameHandler::onDisconnect(JmyEventInfo* info)
{
	ServerLogInfo("ondisconnect conn_id(%d)", info->conn_id);
	return 0;
}

int GameHandler::onTick(JmyEventInfo* info)
{
	(void)info;
	int res = conn_pool_.run();
	return res;
}

int GameHandler::onConnectDBRequest(JmyMsgInfo* info)
{
	if (game_mgr_.getAgentByConnId(info->session_id)) {
		ServerLogError("already exist game agent by conn_id(%d)", info->session_id);
		return -1;
	}
	MsgGS2DS_ConnectDBRequest request;
	if (!request.ParseFromArray(info->data, info->len)) {
		ServerLogError("parse MsgGS2DS_ConnectDBRequest failed");
		return -1;
	}
	int game_id = request.game_id();
	GameAgent* agent = game_mgr_.newAgent(game_id, (JmyTcpConnectionMgr*)info->param, info->session_id);
	if (!agent) {
		ServerLogError("create game agent with game_id(%d), conn_id(%d) failed", game_id, info->session_id);
		return -1;
	}

	MsgDS2GS_ConnectDBResponse response;
	if (!response.SerializeToArray(tmp_, sizeof(tmp_))) {
		ServerLogError("serialize MsgDS2GS_ConnectDBResponse failed");
		return -1;
	}
	if (agent->sendMsg(MSGID_DS2GS_CONNECT_DB_RESPONSE, tmp_, response.ByteSize()) < 0) {
		ServerLogError("send MsgDS2GS_ConnectDBResponse to game server %d failed", game_id);
		return -1;
	}

	ServerLogInfo("game server %d connected", game_id);
	return info->len;
}

int GameHandler::onRequireUserDataRequest(JmyMsgInfo* info)
{
	GameAgent* agent = game_mgr_.getAgentByConnId(info->session_id);
	if (!agent) {
		ServerLogError("not found game agent with conn_id(%d)", info->session_id);
		return -1;
	}
	int id = 0;
	if (!game_mgr_.getIdByConnId(info->session_id, id)) {
		ServerLogError("cant get game_id by conn_id(%d)");
		return -1;
	}

	MsgGS2DS_RequireUserDataRequest request;
	if (!request.ParseFromArray(info->data, info->len)) {
		ServerLogError("parse MsgGS2DS_RequireUserDataRequest failed");
		return -1;
	}

	UserData* user = user_mgr_.get(request.account());
	if (!user) {
		std::pair<std::set<std::string>::iterator, bool> r = accounts_set_.insert(std::move(request.account()));
		std::snprintf(tmp_, sizeof(tmp_), "SELECT * FROM player WHERE `account`=%s", request.account().c_str());
		// to load data from database
		MysqlConnectorPool::CmdInfo cmd;
		cmd.sql = tmp_;
		cmd.sql_len = strlen(tmp_);
		cmd.callback_func = getPlayerInfoCallback;
		cmd.param = (void*)(&r.first);
		cmd.param_l = 0;
		conn_pool_.push_read_cmd(cmd);
	} else {
		
	}

	return info->len;
}

int GameHandler::getPlayerInfoCallback(MysqlConnector::Result& res, void* param, long param_l)
{
	const std::string& account = *(std::string*)param;
	std::set<std::string>::iterator it = accounts_set_.find(account);
	if (it == accounts_set_.end()) {
		ServerLogError("cant found account %s", account.c_str());
		return -1;
	}
	return 0;
}
