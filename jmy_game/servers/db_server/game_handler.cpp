#include "game_handler.h"
#include "../common/util.h"
#include "../../proto/src/server.pb.h"
#include "game_server_manager.h"
#include "user_data_manager.h"
#include "dbres_callback_funcs.h"
#include "global_data.h"
#include "db_server.h"

char GameHandler::tmp_[JMY_MAX_MSG_SIZE];

int GameHandler::onConnect(JmyEventInfo* info)
{
	LogInfo("onconnection conn_id(%d)", info->conn_id);
	return 0;
}

int GameHandler::onDisconnect(JmyEventInfo* info)
{
	LogInfo("ondisconnect conn_id(%d)", info->conn_id);
	return 0;
}

int GameHandler::onTick(JmyEventInfo* info)
{
	(void)info;
	return 0;
}

int GameHandler::processConnectDBRequest(JmyMsgInfo* info)
{
	if (GAME_MGR->getByConnId(info->conn_id)) {
		LogError("already exist game agent by conn_id(%d)", info->conn_id);
		return -1;
	}
	MsgGS2DS_ConnectDBRequest request;
	if (!request.ParseFromArray(info->data, info->len)) {
		LogError("parse MsgGS2DS_ConnectDBRequest failed");
		return -1;
	}
	int game_id = request.game_id();
	GameAgent* agent = GAME_MGR->newAgent(game_id, (JmyTcpConnectionMgr*)info->param, info->conn_id);
	if (!agent) {
		LogError("create game agent with game_id(%d), conn_id(%d) failed", game_id, info->conn_id);
		return -1;
	}

	MsgDS2GS_ConnectDBResponse response;
	if (!response.SerializeToArray(tmp_, sizeof(tmp_))) {
		LogError("serialize MsgDS2GS_ConnectDBResponse failed");
		return -1;
	}
	if (agent->sendMsg(MSGID_DS2GS_CONNECT_DB_RESPONSE, tmp_, response.ByteSize()) < 0) {
		LogError("send MsgDS2GS_ConnectDBResponse to game server %d failed", game_id);
		return -1;
	}

	LogInfo("game server %d connected", game_id);
	return info->len;
}

int GameHandler::processRequireUserDataRequest(JmyMsgInfo* info)
{
	GameAgent* agent = GAME_MGR->getByConnId(info->conn_id);
	if (!agent) {
		LogError("not found game agent with conn_id(%d)", info->conn_id);
		return -1;
	}
	int id = GAME_MGR->getIdByConnId(info->conn_id);
	if (!id) {
		LogError("cant get game_id by conn_id(%d)");
		return -1;
	}

	MsgGS2DS_RequireUserDataRequest request;
	if (!request.ParseFromArray(info->data, info->len)) {
		LogError("parse MsgGS2DS_RequireUserDataRequest failed");
		return -1;
	}

	UserData* user = USER_MGR->get(request.account());
	if (!user) {
		// to load data from database
		std::string& a = const_cast<std::string&>(GLOBAL_DATA->getAccount(request.account()));
		if (a == "") {
			a = GLOBAL_DATA->insertAccount(request.account());
		}
		/*if (!DB_MGR.insertRecord("player", DBResCBFuncs::getPlayerInfo, (void*)&a, 0)) {
			GLOBAL_DATA->removeAccount(request.account());
			LogError("push db read cmd(%s) failed", tmp_);
			return -1;
		}*/
		LogInfo("pushed db read cmd(%s)", tmp_);
	} else {
		
	}

	LogInfo("processRequireUserDataRequest: account(%s)", request.account().c_str());

	return info->len;
}
