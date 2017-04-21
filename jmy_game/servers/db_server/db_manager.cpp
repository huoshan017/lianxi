#include "db_manager.h"
#include "../common/util.h"
#include "db_table_defines.h"
#include "config_loader.h"

DBManager::DBManager()
{
}

DBManager::~DBManager()
{
}

bool DBManager::init()
{
	// init mysql connector pool
	MysqlConnPoolConfig conn_pool_config(
			const_cast<char*>(SERVER_CONFIG.mysql_host.c_str()),
			const_cast<char*>(SERVER_CONFIG.mysql_user.c_str()),
			const_cast<char*>(SERVER_CONFIG.mysql_password.c_str()),
			const_cast<char*>(SERVER_CONFIG.mysql_dbname.c_str()),
			const_cast<MysqlDatabaseConfig*>(&s_jmy_game_db_config));
	if (!conn_pool_.init(conn_pool_config)) {
		LogError("init mysql connector pool failed");
		return false;
	}
	return true;
}

void DBManager::clear()
{
	conn_pool_.close();
}

int DBManager::run()
{
	return conn_pool_.run();
}

bool DBManager::pushReadCmd(const char* sql, unsigned int sql_len, mysql_cmd_callback_func cb, void* param, long param_l)
{
	MysqlConnectorPool::CmdInfo cmd;
	cmd.sql = const_cast<char*>(sql);
	cmd.sql_len = sql_len;
	cmd.callback_func = cb;
	cmd.param = param;
	cmd.param_l = param_l;
	return conn_pool_.push_read_cmd(cmd);
}

bool DBManager::pushWriteCmd(const char* sql, unsigned int sql_len, mysql_cmd_callback_func cb, void* param, long param_l)
{
	MysqlConnectorPool::CmdInfo cmd;
	cmd.sql = const_cast<char*>(sql);
	cmd.sql_len = sql_len;
	cmd.callback_func = cb;
	cmd.param = param;
	cmd.param_l = param_l;
	return conn_pool_.push_write_cmd(cmd);
}
