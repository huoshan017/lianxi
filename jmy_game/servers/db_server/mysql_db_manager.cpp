#include "mysql_db_manager.h"
#include "config_loader.h"

MysqlDBManager::MysqlDBManager() : index_(0), big_index_(0)
{
}

MysqlDBManager::~MysqlDBManager()
{
}

bool MysqlDBManager::init(const MysqlDatabaseConfig& config)
{
	// init mysql connector pool
	MysqlConnPoolConfig conn_pool_config(
			const_cast<char*>(SERVER_CONFIG.mysql_host.c_str()),
			const_cast<char*>(SERVER_CONFIG.mysql_user.c_str()),
			const_cast<char*>(SERVER_CONFIG.mysql_password.c_str()),
			const_cast<char*>(SERVER_CONFIG.mysql_dbname.c_str()),
			const_cast<MysqlDatabaseConfig*>(&config));
	if (!conn_pool_.init(conn_pool_config)) {
		LogError("init mysql_connector_pool (db_name: %s) failed", config.dbname);
		return false;
	}

	if (!config_mgr_.init(config)) {
		LogError("init mysql_db_config_manager (db_name: %s) failed", config.dbname);
		return false;
	}

	LogInfo("init mysql_db_config_manager (db_name: %s) success", config.dbname);
	return true;
}

void MysqlDBManager::clear()
{
	config_mgr_.clear();
	conn_pool_.close();
}

int MysqlDBManager::run()
{
	return conn_pool_.run();
}

bool MysqlDBManager::insertRecord(const char* table_name, mysql_cmd_callback_func get_last_insert_id_func, void* param, long param_l)
{
	int idx = config_mgr_.get_table_index(table_name);
	if (idx < 0) {
		LogError("get table(%s) index failed", table_name);
		return false;
	}
	return insertRecord(idx, get_last_insert_id_func, param, param_l);
}

bool MysqlDBManager::insertRecord(int table_index, mysql_cmd_callback_func get_last_insert_id_func, void* param, long param_l)
{
	const MysqlTableInfo* ti = config_mgr_.get_table_info(table_index);
	if (!ti) {
		LogError("get table(%d) info failed", table_index);
		return false;
	}

	std::snprintf(buf_[0], sizeof(buf_[0]), "INSERT INTO %s", ti->name);
	return push_insert_cmd(buf_[0], strlen(buf_[0]), get_last_insert_id_func, param, param_l);
}

bool MysqlDBManager::push_read_cmd(const char* sql, unsigned int sql_len, mysql_cmd_callback_func get_result_func, void* user_param, long user_param_l)
{
	MysqlConnectorPool::CmdInfo cmd;
	cmd.sql = const_cast<char*>(sql);
	cmd.sql_len = sql_len;
	cmd.callback_func = get_result_func;
	cmd.user_param = user_param;
	cmd.user_param_l = user_param_l;
	cmd.write_cmd = false;
	return conn_pool_.push_read_cmd(cmd);
}

bool MysqlDBManager::push_write_cmd(const char* sql, unsigned int sql_len)
{
	MysqlConnectorPool::CmdInfo cmd;
	cmd.sql = const_cast<char*>(sql);
	cmd.sql_len = sql_len;
	cmd.callback_func = nullptr;
	cmd.user_param = nullptr;
	cmd.user_param_l = 0;
	cmd.write_cmd = true;
	return conn_pool_.push_write_cmd(cmd);
}

bool MysqlDBManager::push_insert_cmd(const char* sql, unsigned int sql_len, mysql_cmd_callback_func get_last_insert_id_func, void* param, long param_l)
{
	if (!push_write_cmd(sql, sql_len))
		return false;

	return push_get_last_insert_id_cmd(get_last_insert_id_func, param, param_l);
}

bool MysqlDBManager::push_get_last_insert_id_cmd(mysql_cmd_callback_func get_last_insert_id_func, void* param, long param_l)
{
	MysqlConnectorPool::CmdInfo cmd;
	cmd.sql = const_cast<char*>("SELECT LAST_INSERT_ID()");
	cmd.sql_len = std::strlen(cmd.sql);
	cmd.callback_func = get_last_insert_id_func;
	cmd.user_param = param;
	cmd.user_param_l = param_l;
	cmd.write_cmd = false;
	return conn_pool_.push_write_cmd(cmd);
}
