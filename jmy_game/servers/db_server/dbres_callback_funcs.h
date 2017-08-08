#pragma once

#include "../mysql/mysql_connector.h"

class t_player;
class DBResCBFuncs {
public:
	// callback functions
	static int getAllAccounts(MysqlConnector::Result& res, void* param, long param_l);
	static int getAccountBaseInfo(MysqlConnector::Result& res, void* param, long param_l);
	static int getPlayerInfo(MysqlConnector::Result& res, void* param, long param_l);

	static int sendGetRoleResponse(t_player* user, int conn_id);
	static int sendGetRoleEmptyResponse(const std::string& account, int conn_id);
private:
	static char tmp_[4096*128];
};
