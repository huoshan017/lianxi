#include "dbres_callback_funcs.h"
#include "../common/util.h"
#include "../../proto/src/server.pb.h"
#include "user_data_manager.h"
#include "global_data.h"
#include "db_server.h"
#include "game_server_manager.h"
#include "db_tables_func.h"
#include <string>

char DBResCBFuncs::tmp_[4096*128];

static int send_game_msg_by_game_id_and_account(int game_id, const std::string& account, int msg_id, const char* data, unsigned short len) {
	GameAgent* game_server = GAME_MGR->get(game_id);
	if (!game_server) {
		LogError("get game server failed by id(%d)", game_id);
		return -1;
	}

	int user_id = GLOBAL_DATA->getUserIdByAccount(account);
	if (user_id <= 0) {
		LogError("get user id by account(%s) failed", account.c_str());
		return -1;
	}

	int res = game_server->sendMsg(user_id, msg_id, data, len);
	if (res < 0) {
		LogError("send msg MsgDS2GS_RequireUserDataResponse failed");
		return -1;
	}
	return res;
}

int DBResCBFuncs::getAllAccounts(MysqlConnector::Result& res, void* param, long param_l)
{
	(void)param;
	(void)param_l;
	if (res.res_err != 0) {
		LogError("get all accounts failed, err(%d)", res.res_err);
		return -1;
	}

	int num = res.num_rows();
	if (num == 0 || res.is_empty()) {
		LogInfo("get all accounts result is empty");
		return 0;
	}

	char** datas = nullptr;
	while (true) {
		datas = res.fetch();
		if (!datas) break;
		if (res.num_fields() < 1) {
			LogError("error: result field num less 1");
			return -1;
		}
		const char* account = datas[0];
		GLOBAL_DATA->insertDBAccount(std::string(account));
	}

	LogInfo("accounts num is %d", num);

	return 0;
}

int DBResCBFuncs::insertPlayerInfo(MysqlConnector::Result& res, void* param, long param_l)
{
	const std::string& account = *(std::string*)param;
	int game_id = (int)param_l;
	if (!GLOBAL_DATA->findAccount(account)) {
		LogError("cant found account %s", account.c_str());
		return -1;
	}

	UserData* user = USER_MGR->get(account);
	if (user) {
		LogWarn("account(%s) already exists", account.c_str());
		return 0;
	}

	int id = mysql_get_last_insert_id(res);
	if (id <= 0) {
		LogError("account(%s) get last insert id invalid", account.c_str());
		return -1;
	}
	uint64_t uid = id + (((uint64_t)game_id<<32)&0xffffffff00000000);

	MysqlFieldNameValue<uint64_t> nv("uid", uid);
	if (!DB_MGR.updateRecord("t_player", "account", account, nv)) {
		LogError("update player account(%s) uid(%llu) failed", account.c_str(), uid);
		return -1;
	}

	user = USER_MGR->getFree(account);
	if (!user) {
		LogError("account(%s) get free user failed", account.c_str());
		return -1;
	}

	user->game_server_id = (int)param_l;

	GLOBAL_DATA->insertDBAccount(account);
	
	MsgDS2GS_RequireUserDataResponse response;
	response.set_account(account);
	response.mutable_user_data()->set_id(id);
	response.mutable_user_data()->set_uid(uid);
	if (!response.SerializeToArray(tmp_, sizeof(tmp_))) {
		LogError("serialize MsgDS2GS_RequireUserDataResponse failed");
		return -1;
	}

	if (send_game_msg_by_game_id_and_account(user->game_server_id, account, MSGID_DS2GS_REQUIRE_USER_DATA_RESPONSE, tmp_, response.ByteSize()) < 0) {
		LogError("send msg MsgDS2GS_RequireUserDataResponse by game_id(%d) and account(%s) failed", user->game_server_id, account.c_str());
		return -1;
	}
	
	LogInfo("insertPlayerInfo");
	return 0;
}

int DBResCBFuncs::getPlayerInfo(MysqlConnector::Result& res, void* param, long param_l)
{
	const std::string& account = *(std::string*)param;
	if (!GLOBAL_DATA->findAccount(account)) {
		LogError("cant found account %s", account.c_str());
		return -1;
	}

	UserData* user = USER_MGR->getFree(account);
	if (!user) {
		LogError("cant get free UserData by account(%s)", account.c_str());
		return -1;
	}

	if (!db_get_result_of_select_t_player(res, user->player_data)) {
		LogError("get result of select player info failed");
		return -1;
	}

	MsgDS2GS_RequireUserDataResponse response;
	response.set_account(account);
	response.mutable_user_data()->set_id(user->player_data.get_id());
	response.mutable_user_data()->set_uid(user->player_data.get_uid());
	response.mutable_user_data()->set_nick_name(user->player_data.get_nick_name());
	response.mutable_user_data()->set_sex(user->player_data.get_sex());
	response.mutable_user_data()->set_exp(user->player_data.get_exp());
	response.mutable_user_data()->set_level(user->player_data.get_level());
	response.mutable_user_data()->set_vip_level(user->player_data.get_vip_level());
	response.mutable_user_data()->set_allocated_items(&user->player_data.get_mutable_items());
	response.mutable_user_data()->set_allocated_skills(&user->player_data.get_mutable_skills());
	response.mutable_user_data()->set_allocated_tasks(&user->player_data.get_mutable_tasks());
	response.mutable_user_data()->set_allocated_daily_tasks(&user->player_data.get_mutable_daily_tasks());
	response.mutable_user_data()->set_allocated_activities(&user->player_data.get_mutable_activities());
	if (!response.SerializeToArray(tmp_, sizeof(tmp_))) {
		LogError("serialize MsgDS2GS_RequireUserDataResponse failed");
		return -1;
	}

	int game_id = (int)param_l;
	if (send_game_msg_by_game_id_and_account(game_id, account, MSGID_DS2GS_REQUIRE_USER_DATA_RESPONSE, tmp_, response.ByteSize()) < 0) {
		LogError("send MsgDS2GS_RequireUserDataResponse by game_id(%d) and account(%s) failed", game_id, account.c_str());
		return -1;
	}

	LogInfo("getPlayerInfo");
	return 0;
}
