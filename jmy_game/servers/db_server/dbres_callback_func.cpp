#include "dbres_callback_funcs.h"
#include "../common/util.h"
#include "user_data_manager.h"
#include "global_data.h"
#include <string>

char DBResCBFuncs::tmp_[4096];

int DBResCBFuncs::getPlayerInfo(MysqlConnector::Result& res, void* param, long param_l)
{
	(void)param_l;
	const std::string& account = *(std::string*)param;
	if (!GLOBAL_DATA->findAccount(account)) {
		LogError("cant found account %s", account.c_str());
		return -1;
	}
	if (res.num_rows()==0 || res.is_empty()) {
		UserData* user = USER_MGR->get(account);
		if (!user) {
			LogError("cant found user agent by account %s", account.c_str());
			return -1;
		}
		user->account = account;
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
