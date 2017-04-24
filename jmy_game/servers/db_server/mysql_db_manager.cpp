#include "mysql_db_manager.h"

const char* MysqlDBManager::s_last_insert_id_str_ = "SELECT LAST_INSERT_ID";

MysqlDBManager::MysqlDBManager()
{
}

MysqlDBManager::~MysqlDBManager()
{
}

bool MysqlDBManager::init(const MysqlDatabaseConfig& config)
{
	return true;
}

void MysqlDBManager::clear()
{
}

int MysqlDBManager::run()
{
	return 0;
}

bool MysqlDBManager::insertRecord(const char* table_name, mysql_cmd_callback_func get_last_insert_id_func)
{
	std::unordered_map<std::string, int>::iterator it = table_name2index_.find(std::string(table_name));
	if (it == table_name2index_.end()) {
		return false;
	}
	int idx = it->second;
	return insertRecord(idx, get_last_insert_id_func);
}

bool MysqlDBManager::insertRecord(int table_index, mysql_cmd_callback_func get_last_insert_id_func)
{
	if (table_index<0 || table_index>=(int)table_array_.size() || table_index>=(int)table_fields_name2index_array_.size())
		return false;
	
	MysqlTableInfo* ti = table_array_[table_index];
	if (!ti)
		return false;

	std::snprintf(buf_[0], sizeof(buf_[0]), "INSERT INTO %s", ti->name);
	if (!push_write_cmd(buf_[0], strlen(buf_[0]), get_last_insert_id_func, nullptr, 0)) {
		return false;
	}

	return true;
}

bool MysqlDBManager::push_read_cmd(const char* sql, unsigned int sql_len, mysql_cmd_callback_func get_result_func, void* user_param, long user_param_l)
{
	MysqlConnectorPool::CmdInfo cmd;
	cmd.sql = const_cast<char*>(sql);
	cmd.sql_len = sql_len;
	cmd.callback_func = get_result_func;
	cmd.user_param = user_param;
	cmd.user_param_l = user_param_l;
	return conn_pool_.push_read_cmd(cmd);
}

bool MysqlDBManager::push_write_cmd(const char* sql, unsigned int sql_len, void* param, long param_l)
{
	MysqlConnectorPool::CmdInfo cmd;
	cmd.sql = const_cast<char*>(sql);
	cmd.sql_len = sql_len;
	cmd.callback_func = nullptr;
	cmd.user_param = param;
	cmd.user_param_l = param_l;
	return conn_pool_.push_write_cmd(cmd);
}

bool MysqlDBManager::push_write_cmd(const char* sql, unsigned int sql_len, mysql_cmd_callback_func get_last_insert_id_func, void* param, long param_l)
{
	if (!push_write_cmd(sql, sql_len, param, param_l))
		return false;
	return push_read_cmd(s_last_insert_id_str_, strlen(s_last_insert_id_str_), get_last_insert_id_func, nullptr, 0);
}

int MysqlDBManager::get_field_type(int table_index, const char* field_name, const char** table_name)
{
	if (table_index<0 || table_index>=(int)table_array_.size() || table_index>=(int)table_fields_name2index_array_.size())
		return false;
	
	MysqlTableInfo* ti = table_array_[table_index];
	if (!ti)
		return false;

	table_fields_name2index_type& tf = table_fields_name2index_array_[table_index];	
	table_fields_name2index_type::iterator it = tf.find(field_name);
	if (it == tf.end()) {
		return false;
	}

	int idx = it->second;
	int ft = ti->fields_info[idx].field_type;

	if (table_name) {
		*table_name = ti->name;
	}

	return ft;
}

