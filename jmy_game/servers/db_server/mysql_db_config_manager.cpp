#include "mysql_db_config_manager.h"
#include "mysql_util.h"
#include "../common/util.h"

MysqlDBConfigManager::MysqlDBConfigManager()
{
}

MysqlDBConfigManager::~MysqlDBConfigManager()
{
	clear();
}

bool MysqlDBConfigManager::init(const MysqlDatabaseConfig& config)
{
	if (config.tables_num <= 0) return false;

	int i = 0;
	for (; i<config.tables_num; ++i) {
		table_name2index_.insert(std::make_pair(std::string(config.tables_info[i].name), i));
		LogInfo("insert table(name:%s, index:%d), size(%d) table_name2index_(0x%x)",
				config.tables_info[i].name, i, table_name2index_.size(), &table_name2index_);
	}

	table_array_.resize(config.tables_num);
	for (i=0; i<config.tables_num; ++i) {
		table_array_[i] = &config.tables_info[i];
	}

	table_fields_name2index_array_.resize(config.tables_num);
	for (i=0; i<config.tables_num; ++i) {
		int s = config.tables_info[i].fields_num;
		for (int j=0; j<s; ++j) {
			table_fields_name2index_array_[i].insert(std::make_pair(config.tables_info[i].fields_info[j].name, j));
		}
	}

	db_name_ = config.dbname;
	return true;
}

void MysqlDBConfigManager::clear()
{
	table_name2index_.clear();
	table_array_.clear();
	table_fields_name2index_array_.clear();
}

int MysqlDBConfigManager::get_table_index(const char* table_name)
{
	std::unordered_map<std::string, int>::iterator it = table_name2index_.find(std::string(table_name));
	if (it == table_name2index_.end()) {
		return -1;
	}
	return it->second;
}

int MysqlDBConfigManager::get_table_field_index(int table_index, const char* field_name)
{
	table_fields_name2index_type& tf = table_fields_name2index_array_[table_index];
	table_fields_name2index_type::iterator it = tf.find(field_name);
	if (it == tf.end())
		return -1;
	return it->second;
}

const MysqlTableInfo* MysqlDBConfigManager::get_table_info(int table_index)
{
	if (table_index<0 || table_index>=(int)table_array_.size() || table_index>=(int)table_fields_name2index_array_.size())
		return nullptr;
	
	const MysqlTableInfo* ti = table_array_[table_index];
	return ti;
}

const MysqlTableFieldInfo* MysqlDBConfigManager::get_field_info(int table_index, const char* field_name)
{
	const MysqlTableInfo* ti = get_table_info(table_index);
	if (!ti) return nullptr;

	table_fields_name2index_type& tf = table_fields_name2index_array_[table_index];	
	table_fields_name2index_type::iterator it = tf.find(field_name);
	if (it == tf.end())
		return nullptr;

	int idx = it->second;
	return &ti->fields_info[idx];
}
