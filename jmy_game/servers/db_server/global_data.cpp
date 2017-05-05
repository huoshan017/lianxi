#include "global_data.h"
#include "db_server.h"
#include "mysql_db_manager.h"
#include "dbres_callback_funcs.h"

bool GlobalData::init()
{
	std::list<const char*> l;
	l.push_back("account");
	if (!DB_MGR.selectRecord("t_player", l, DBResCBFuncs::getAllAccounts, nullptr, 0)) {
		return false;
	}
	LogInfo("to get all accounts");
	return true;
}
