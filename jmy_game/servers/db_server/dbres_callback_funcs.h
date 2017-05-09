#pragma once

#include "mysql_connector.h"

class DBResCBFuncs {
public:
	// callback functions
	static int getAllAccounts(MysqlConnector::Result& res, void* param, long param_l);
	static int getPlayerInfo(MysqlConnector::Result& res, void* param, long param_l);
	static int insertPlayerInfo(MysqlConnector::Result& res, void* param, long param_l);
	static int getPlayerUid(MysqlConnector::Result& res, void* param, long param_l);

private:
	static char tmp_[4096*128];
};
