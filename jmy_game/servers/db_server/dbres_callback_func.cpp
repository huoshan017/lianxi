#include "dbres_callback_funcs.h"
#include "../common/util.h"
#include "user_data_manager.h"
#include "global_data.h"
#include "db_server.h"
#include <string>

char DBResCBFuncs::tmp_[4096];

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

	if (res.res_err != 0) {
		LogError("insert player(%s) info failed, err(%d)", account.c_str(), res.res_err);
		return -1;
	}

	if (res.num_rows() == 0 || res.is_empty()) {
		LogError("insert player(%s) result is empty", account.c_str());
		return -1;	
	}

	char** datas = res.fetch();
	if (!datas) {
		LogError("cant get data from result");
		return -1;
	}

	int id = std::atoi(datas[0]);
	uint64_t uid = id + (((uint64_t)game_id<<32)&0xffffffff00000000);

	MysqlFieldNameValue<uint64_t> nv(std::string("uid"), uid);
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
	user->player_data.uid = uid;
	user->player_data.id = id;
	user->player_data.account = account;
	
	LogInfo("insertPlayerInfo");
	return 0;
}

int DBResCBFuncs::getPlayerInfo(MysqlConnector::Result& res, void* param, long param_l)
{
	(void)param_l;
	const std::string& account = *(std::string*)param;
	if (!GLOBAL_DATA->findAccount(account)) {
		LogError("cant found account %s", account.c_str());
		return -1;
	}

	if (res.res_err != 0) {
		LogError("getPlayerInfo(account:%s) result(%d) is not no error", account.c_str(), res.res_err);
		return -1;
	}

	if (res.num_rows()==0 || res.is_empty()) {
		UserData* user = USER_MGR->get(account);
		if (!user) {
			LogError("cant found user agent by account %s", account.c_str());
			return -1;
		}
		std::snprintf(tmp_, sizeof(tmp_), "INSERT ");
	} else {
		char** datas = res.fetch();
		int i = 0;
		while (datas) {
			for (i=0; i<res.num_fields(); ++i) {
			}
			datas = res.fetch();
		}
		res.clear();
	}

	LogInfo("getPlayerInfo");

	return 0;
}
