#include "game_handler.h"
#include "../common/util.h"
#include "../../proto/src/server.pb.h"
#include "game_server_manager.h"
#include "user_data_manager.h"
#include "dbres_callback_funcs.h"
#include "global_data.h"
#include "db_server.h"
#include "mysql_defines.h"
#include "db_struct_funcs.h"

char GameHandler::tmp_[JMY_MAX_MSG_SIZE];
t_player GameHandler::tmp_player_;

int GameHandler::onConnect(JmyEventInfo* info)
{
	LogInfo("onconnection conn_id(%d)", info->conn_id);
	return 0;
}

int GameHandler::onDisconnect(JmyEventInfo* info)
{
	if (GAME_MGR->getByConnId(info->conn_id)) {
		GAME_MGR->removeByConnId(info->conn_id);
	}
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
	GameAgent* agent = GAME_MGR->get(game_id);
	if (!agent) {
		agent = GAME_MGR->newAgent(game_id, (JmyTcpConnectionMgr*)info->param, info->conn_id);
		if (!agent) {
			LogError("create game agent with game_id(%d), conn_id(%d) failed", game_id, info->conn_id);
			return -1;
		}
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

	int game_id = GAME_MGR->getIdByConnId(info->conn_id);
	if (!game_id) {
		LogError("cant get game_id by conn_id(%d)");
		return -1;
	}

	int user_id = info->user_id;
	if (user_id <= 0) {
		LogError("user_id %d is invalid", user_id);
		return -1;
	}

	MsgGS2DS_RequireUserDataRequest request;
	if (!request.ParseFromArray(info->data, info->len)) {
		LogError("parse MsgGS2DS_RequireUserDataRequest failed");
		return -1;
	}

	UserData* user = USER_MGR->get(request.account());
	if (!user) {
		const std::string& a = const_cast<std::string&>(GLOBAL_DATA->getAccount(request.account()));
		GLOBAL_DATA->setAccount2UserId(a, user_id);
		// not found in db, insert new record
		if (!GLOBAL_DATA->findDBAccount(a)) {
			tmp_player_.account = a;
			if (!db_insert_t_player_record(tmp_player_, DBResCBFuncs::insertPlayerInfo, (void*)&a, (long)game_id)) {
				return -1;
			}
			LogInfo("to inserting new record(account:%s)", a.c_str());
		} else {
			if (!db_select_t_player_fields_by_account(a, DBResCBFuncs::getPlayerInfo, (void*)&a, (long)game_id)) {
				LogError("select account(%s) record failed", a.c_str());
				return -1;
			}
			LogInfo("to selecting record by account(%s)", a.c_str());
		}
	} else {
		MsgDS2GS_RequireUserDataResponse response;
		response.set_account(request.account());
		if (!response.SerializeToArray(tmp_, sizeof(tmp_))) {
			LogError("serialize MsgDS2GS_RequireUserDataResponse failed");
			return -1;
		}
		if (agent->sendMsg(MSGID_DS2GS_REQUIRE_USER_DATA_RESPONSE, tmp_, response.ByteSize()) < 0) {
			LogError("send MsgDS2GS_RequireUserDataResponse failed");
			return -1;
		}
	}

	LogInfo("processRequireUserDataRequest: account(%s)", request.account().c_str());

	return info->len;
}
