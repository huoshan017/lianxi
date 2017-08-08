#include "global_data.h"
#include "db_server.h"
#include "../mysql/mysql_db_manager.h"
#include "dbres_callback_funcs.h"

bool GlobalData::init()
{
	std::list<const char*> l;
	l.push_back("account");
	if (!DB_MGR.selectRecord("t_player", l,
		[](MysqlConnector::Result& res) {
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
		}/*, nullptr, 0*/)) {
		return false;
	}
	LogInfo("to get all accounts");
	return true;
}
