#pragma once

#include "../libjmy/jmy_singleton.hpp"
#include "mysql_connector_pool.h"
#include <set>

class DBManager  : public JmySingleton<DBManager>
{
public:
	DBManager();
	~DBManager();

	bool init();
	void clear();
	int run();

	bool pushReadCmd(const char* sql, unsigned int sql_len, mysql_cmd_callback_func cb, void* param, long param_l);
	bool pushWriteCmd(const char* sql, unsigned int sql_len, mysql_cmd_callback_func cb, void* param, long param_l);

private:
	MysqlConnectorPool conn_pool_;
	std::set<std::string> accounts_;
};

#define DB_MGR (DBManager::getInstance())
