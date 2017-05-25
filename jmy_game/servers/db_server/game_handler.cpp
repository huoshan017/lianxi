#include "game_handler.h"
#include "../common/util.h"
#include "../../proto/src/server.pb.h"
#include "game_server_manager.h"
#include "dbres_callback_funcs.h"
#include "global_data.h"
#include "db_server.h"
#include "../mysql/mysql_defines.h"
#include "db_tables_func.h"
#include "db_tables_struct.h"

char GameHandler::tmp_[JMY_MAX_MSG_SIZE];

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

int GameHandler::processGetRole(JmyMsgInfo* info)
{
	MsgGS2DS_GetRoleRequest request;
	if (!request.ParseFromArray(info->data, info->len)) {
		LogError("parse MsgGS2DS_GetRoleRequest failed");
		return -1;
	}

	mysql_records_manager2<t_player, uint64_t, std::string>& player_mgr = TABLES_MGR.get_t_player_table();
	t_player* user = player_mgr.get_by_key2(request.account());
	if (!user) {
		user = player_mgr.get_new_by_key2(request.account());
		user->set_account(request.account());
		if (!db_select_t_player_fields_by_account(user->get_account(), DBResCBFuncs::getPlayerInfo, (void*)&user->get_account(), (long)info->conn_id)) {
			LogError("select account(%s) record failed", user->get_account().c_str());
			return -1;
		}
		LogInfo("to selecting record by account(%s)", user->get_account().c_str());
	} else {
		if (DBResCBFuncs::sendGetRoleResponse(user, info->conn_id) < 0) {
			LogError("send get account(%s) role response failed", request.account().c_str());
			return -1;
		}
		LogInfo("send get account(%s) role response", request.account().c_str());
	}

	return info->len;
}

static inline uint64_t gen_unique_role_id(int server_id) {
	uint64_t id = (std::time(nullptr)<<32) & 0xffffffff00000000;
	id += (server_id<<16)&0xffff0000;
	static uint16_t counter = 0;
	id += counter;
	counter += 1;
	return id;
}

int GameHandler::processCreateRole(JmyMsgInfo* info)
{
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) return -1;

	MsgGS2DS_CreateRoleRequest request;
	if (!request.ParseFromArray(info->data, info->len)) {
		LogError("parse MsgGS2DS_CreateRoleRequest failed");
		return -1;
	}

	mysql_records_manager2<t_player, uint64_t, std::string>& player_mgr = TABLES_MGR.get_t_player_table();
	t_player* user = player_mgr.get_by_key2(request.account());
	if (user) {
		LogError("already exist role(%llu) for account(%s)", user->get_role_id(), request.account().c_str());
		return -1;
	}

	user = player_mgr.get_new_by_key2(request.account());
	user->set_account(request.account());
	int game_id = GAME_MGR->getIdByConnId(info->conn_id);
	uint64_t role_id = gen_unique_role_id(game_id);
	user->set_role_id(role_id);
	user->set_sex(request.sex());
	user->set_nick_name(request.nick_name());

	GLOBAL_DATA->setAccount2UserId(user->get_account(), role_id);
	// not found in db, insert new record
	if (!user->insert_request(nullptr, nullptr, 0)) {
		LogError("insert t_player record for account(%s) failed", user->get_account().c_str());
		return -1;
	}

	MsgDS2GS_CreateRoleResponse response;
	response.set_account(request.account());
	response.mutable_role_data()->set_nick_name(request.nick_name());
	response.mutable_role_data()->set_sex(request.sex());
	response.mutable_role_data()->set_race(request.race());
	response.mutable_role_data()->set_role_id(role_id);
	if (!response.SerializeToArray(tmp_, sizeof(tmp_))) {
		LogError("serialize MsgDS2GS_CreateRoleResponse failed");
		return -1;
	}

	if (conn->send(MSGID_DS2GS_CREATE_ROLE_RESPONSE, tmp_, response.ByteSize()) < 0) {
		LogError("send MsgDS2GS_CreateRoleResponse failed");
		return -1;
	}

	LogInfo("to inserting new record(account:%s)", user->get_account().c_str());

	return info->len;
}
