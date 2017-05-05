#include "game_handler.h"
#include "../common/util.h"
#include "../../proto/src/server.pb.h"
#include "game_server_manager.h"
#include "user_data_manager.h"
#include "dbres_callback_funcs.h"
#include "global_data.h"
#include "db_server.h"
#include "mysql_defines.h"

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
	int game_id = GAME_MGR->getIdByConnId(info->conn_id);
	if (!game_id) {
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
		const std::string& a = const_cast<std::string&>(GLOBAL_DATA->getAccount(request.account()));
		// not found in db, insert new record
		if (!GLOBAL_DATA->findDBAccount(request.account())) {
			MysqlFieldNameValue<const std::string&> account_nv(std::string("account"), a);
			MysqlFieldNameValue<int> level_nv(std::string("level"), 1);
			MysqlFieldNameValue<const std::string&> nickname_nv(std::string("nick_name"), "");
			MysqlFieldNameValue<int> viplevel_nv(std::string("vip_level"), 0);
			LogInfo("account = %s, request account = %s", a.c_str(), request.account().c_str());
			if (!DB_MGR.insertRecord("t_player", DBResCBFuncs::insertPlayerInfo, (void*)&a, (long)game_id, account_nv, level_nv, nickname_nv, viplevel_nv)) {
				LogError("insert new record(account:%s) failed", a.c_str());
				return -1;
			}
			LogInfo("to inserting new record(account:%s)", a.c_str());
		} else {
			if (!DB_MGR.selectRecord("t_player", "account", a, DBResCBFuncs::getPlayerInfo, (void*)&a, (long)0)) {
				LogError("select account(%s) record failed", a.c_str());
				return -1;
			}
			LogInfo("to selecting record by account(%s)", a.c_str());
		}
	} else {
		MsgDS2GS_RequireUserDataResponse response;
		response.set_account(request.account());
		if (response.SerializeToArray(tmp_, sizeof(tmp_))) {
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
