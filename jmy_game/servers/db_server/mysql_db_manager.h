#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <tuple>
#include <boost/algorithm/string.hpp>
#include "../libjmy/jmy_util.h"
#include "../common/util.h"
#include "mysql_util.h"
#include "mysql_defines.h"
#include "mysql_connector_pool.h"
#include "mysql_db_config_manager.h"

class MysqlDBManager
{
public:
	MysqlDBManager();
	~MysqlDBManager();

	bool init(const MysqlDatabaseConfig& config);
	void clear();
	int run();

	bool insertRecord(const char* table_name, mysql_cmd_callback_func get_last_insert_id_func, void* param, long param_l);
	template <typename... FieldNameValueArgs>
	bool insertRecord(const char* table_name, mysql_cmd_callback_func get_last_insert_id_func, void* param, long param_l, const FieldNameValueArgs&... args);

	bool insertRecord(int table_index, mysql_cmd_callback_func get_last_insert_id_func, void* param, long param_l);
	template <typename... FieldNameValueArgs>
	bool insertRecord(int table_index, const FieldNameValueArgs&... args);
	template <typename... FieldNameValueArgs>
	bool insertRecord(int table_index, mysql_cmd_callback_func get_last_insert_id_func, void* param, long param_l, const FieldNameValueArgs&... args);

	template <typename KeyType, typename... FieldNameValueArgs>
	bool updateRecord(const char* table_name, const char* key_name, const KeyType& key_value, const FieldNameValueArgs&... args);
	template <typename KeyType, typename... FieldNameValueArgs>
	bool updateRecord(int table_index, const char* key_name, const KeyType& key_value, const FieldNameValueArgs&... args);

	template <typename KeyType>
	bool deleteRecord(const char* table_name, const char* key_name, const KeyType& key_value);
	template <typename KeyType>
	bool deleteRecord(int table_index, const char* key_name, const KeyType& key_value);

	template <typename KeyType>
	bool selectRecord(const char* table_name,
			const char* key_name, const KeyType& key_value,
			mysql_cmd_callback_func get_result_func);

	template <typename KeyType, typename KeyType2>
	bool selectRecord(const char* table_name,
			const char* key_name, const KeyType& key_value,
			const char* key2_name, const KeyType2& key2_value,
			mysql_cmd_callback_func get_result_func);

	template <typename KeyType>
	bool selectRecord(const char* table_name,
			const char* key_name, const KeyType& key_value,
			const std::list<const char*>& fields_list,
			mysql_cmd_callback_func get_result_func);

	template <typename KeyType, typename KeyType2>
	bool selectRecord(const char* table_name,
			const char* key_name, const KeyType& key_value,
			const char* key2_name, const KeyType2& key2_value,
			const std::list<const char*>& fields_list,
			mysql_cmd_callback_func get_result_func);

	template <typename KeyType>
	bool selectRecord(int table_index,
			const char* key_name, const KeyType& key_value,
			mysql_cmd_callback_func get_result_func);

	template <typename KeyType, typename KeyType2>
	bool selectRecord(int table_index,
			const char* key_name, const KeyType& key_value,
			const char* key2_name, const KeyType2& key2_value,
			mysql_cmd_callback_func get_result_func);

	template <typename KeyType>
	bool selectRecord(int table_index,
			const char* key_name, const KeyType& key_value,
			const std::list<const char*>& fields_list,
			mysql_cmd_callback_func get_result_func);

	template <typename KeyType, typename KeyType2>
	bool selectRecord(int table_index,
			const char* key_name, const KeyType& key_value,
			const char* key2_name, const KeyType2& key2_value,
			const std::list<const char*>& fields_list,
			mysql_cmd_callback_func get_result_func);

private:
	bool push_read_cmd(const char* sql, unsigned int sql_len, mysql_cmd_callback_func get_result_func, void* param, long param_l);
	bool push_write_cmd(const char* sql, unsigned int sql_len);
	bool push_insert_cmd(const char* sql, unsigned int sql_len, mysql_cmd_callback_func get_last_insert_id_func, void* param, long param_l);
	bool push_get_last_insert_id_cmd(mysql_cmd_callback_func get_last_insert_id_func, void* param, long param_l);

private:
	MysqlDBConfigManager config_mgr_;
	MysqlConnectorPool conn_pool_;
	char buf_[2][1024*128];
};


template <typename... FieldNameValueArgs>
bool MysqlDBManager::insertRecord(const char* table_name, mysql_cmd_callback_func get_last_insert_id_func, void* param, long param_l, const FieldNameValueArgs&... args)
{
	int idx = config_mgr_.get_table_index(table_name);
	if (idx < 0) {
		LogError("get table(%s) index failed", table_name);
		return false;
	}
	return insertRecord(idx, get_last_insert_id_func, param, param_l, args...);
}

template <typename... FieldNameValueArgs>
bool MysqlDBManager::insertRecord(int table_index, mysql_cmd_callback_func get_last_insert_id_func, void* param, long param_l, const FieldNameValueArgs&... args)
{
	const MysqlTableInfo* ti = config_mgr_.get_table_info(table_index);
	if (!ti) {
		LogError("get table(%d) index failed", table_index);
		return false;
	}

	const std::tuple<FieldNameValueArgs...> t(args...);
	size_t s = sizeof...(FieldNameValueArgs);
	size_t i = 0;

	std::string types_str;
	char* tmp_values_str = nullptr;
	int bi = 0; int bi_max = 1;
	for (; i<s; ++i) {
		auto v = std::get<i>(t);
		int idx = config_mgr_.get_table_field_index(table_index, v.field_name);
		if (idx < 0) {
			LogError("not found field_name(%s)", v.field_name);
			return false;
		}

		int ft = ti->fields_info[idx].field_type;
		int flags = ti->fields_info[idx].create_flags;
		const char* format = mysql_get_field_type_format((MysqlTableFieldType)ft, flags);
		if (!format) {
			LogError("field_type(%d), create_flags(%d) get format error", ft, flags);
			return false;
		}
		if (!tmp_values_str) {
			std::snprintf(buf_[bi], sizeof(buf_[bi]), "%s", format);
		} else {
			std::snprintf(buf_[bi], sizeof(buf_[bi]), "%s, %s", tmp_values_str, format);
		}
		tmp_values_str = buf_[bi];
		bi += 1;
		if (bi > bi_max) bi = 0;
	}

	std::snprintf(buf_[bi], sizeof(buf_[bi]), "INSERT INTO %s (%s) VALUES (%s)", ti->name, types_str.c_str(), tmp_values_str);
	return push_insert_cmd(buf_[bi], strlen(buf_[bi]), get_last_insert_id_func, param, param_l);
}

template <typename KeyType, typename... FieldNameValueArgs>
bool MysqlDBManager::updateRecord(const char* table_name, const char* key_name, const KeyType& key_value, const FieldNameValueArgs&... args)
{
	int idx = config_mgr_.get_table_index(table_name);
	if (idx < 0) {
		LogError("get table(%s) index failed", table_name);
		return false;
	}
	return updateRecord(idx, key_name, key_value, args...);
}

template <typename KeyType, typename... FieldNameValueArgs>
bool MysqlDBManager::updateRecord(int table_index, const char* key_name, const KeyType& key_value, const FieldNameValueArgs&... args)
{
	const MysqlTableInfo* ti = config_mgr_.get_table_info(table_index);
	if (!ti) {
		LogError("get table(%d) info failed", table_index);
		return false;
	}

	std::tuple<FieldNameValueArgs...> t(args...);
	size_t s = sizeof...(FieldNameValueArgs);
	size_t i = 0;

	char* tmp_values_str = nullptr;
	int bi = 0; int bi_max = 1;
	for (; i<s; ++i) {
		auto v = std::get<i>(t);
		int idx = config_mgr_.get_table_field_index(table_index, v.field_name);
		if (idx < 0) {
			LogError("not found field_name(%s)", v.field_name);
			return false;
		}

		// field type and value
		int ft = ti->fields_info[idx].field_type;
		int flags = ti->fields_info[idx].create_flags;
		const char* format = mysql_get_field_type_format((MysqlTableFieldType)ft, flags);
		if (!format) {
			LogError("field_type(%d) create_flags(%d) get format failed", ft, flags);
			return false;
		}
		if (!tmp_values_str) {
			std::snprintf(buf_[bi], sizeof(buf_[bi]), "%s=%s", v.field_name, format);
		} else {
			std::snprintf(buf_[bi], sizeof(buf_[bi]), "%s, %s=%s", tmp_values_str, v.field_name, format);
		}
		tmp_values_str = buf_[bi];
		bi += 1;
		if (bi > bi_max) bi = 0;
	}

	const char* format = config_mgr_.get_field_type_format(table_index, key_name);
	if (!format) {
		LogError("table_index(%d) key_name(%s) get format failed", table_index, key_name);
		return false;
	}
	std::snprintf(buf_[bi], sizeof(buf_[bi]), "UPDATE %s SET %s WHERE %s=%s", ti->name, tmp_values_str, key_name, format);
	return push_write_cmd(buf_[bi], strlen(buf_[bi]));
}

template <typename KeyType>
bool MysqlDBManager::deleteRecord(const char* table_name, const char* key_name, const KeyType& key_value)
{
	int idx = config_mgr_.get_table_index(table_name);
	if (idx < 0) {
		LogError("get table(%s) index failed", table_name);
		return false;
	}
	return deleteRecord(idx, key_name, key_value);
}

template <typename KeyType>
bool MysqlDBManager::deleteRecord(int table_index, const char* key_name, const KeyType& key_value)
{
	const MysqlTableInfo* ti = config_mgr_.get_table_info(table_index);
	if (!ti) {
		LogError("get table(%d) info failed", table_index);
		return false;
	}
	const char* format = config_mgr_.get_field_type_format(table_index, key_name);
	if (!format) {
		LogError("table_index(%d) key_name(%s) get field_type format failed", table_index, key_name);
		return false;
	}
	std::snprintf(buf_[0], sizeof(buf_[0]), "DELETE FROM %s WHERE %s=%s", ti->name, key_name, format);
	return push_write_cmd(buf_[0], std::strlen(buf_[0]));
}

template <typename KeyType>
bool MysqlDBManager::selectRecord(
		const char* table_name,
		const char* key_name, const KeyType& key_value,
		mysql_cmd_callback_func get_result_func)
{
	int idx = config_mgr_.get_table_index(table_name);
	if (idx < 0) {
		LogError("get table(%s) index failed", table_name);
		return false;
	}
	return selectRecord(idx, key_name, key_value, get_result_func);
}

template <typename KeyType, typename KeyType2>
bool MysqlDBManager::selectRecord(
		const char* table_name,
		const char* key_name, const KeyType& key_value,
		const char* key2_name, const KeyType2& key2_value,
		mysql_cmd_callback_func get_result_func)
{
	int idx = config_mgr_.get_table_index(table_name);
	if (idx < 0) {
		LogError("get table(%s) index failed", table_name);
		return false;
	}
	return selectRecord(idx, key_name, key_value, key2_name, key2_value, get_result_func);
}

template <typename KeyType>
bool MysqlDBManager::selectRecord(
		const char* table_name,
		const char* key_name, const KeyType& key_value,
		const std::list<const char*>& fields_list,
		mysql_cmd_callback_func get_result_func)
{
	int idx = config_mgr_.get_table_index(table_name);
	if (idx < 0) {
		LogError("get table(%s) index failed", table_name);
		return false;
	}
	return selectRecord(idx, key_name, key_value, fields_list, get_result_func);
}

template <typename KeyType, typename KeyType2>
bool MysqlDBManager::selectRecord(
		const char* table_name,
		const char* key_name, const KeyType& key_value,
		const char* key2_name, const KeyType2& key2_value,
		const std::list<const char*>& fields_list,
		mysql_cmd_callback_func get_result_func)
{
	int idx = config_mgr_.get_table_index(table_name);
	if (idx < 0) {
		LogError("get table(%s) index failed", table_name);
		return false;
	}
	return selectRecord(idx, key_name, key_value, key2_name, key2_value, fields_list, get_result_func);
}

template <typename KeyType>
bool MysqlDBManager::selectRecord(
		int table_index,
		const char* key_name, const KeyType& key_value,
		mysql_cmd_callback_func get_result_func)
{
	const MysqlTableInfo* ti = config_mgr_.get_table_info(table_index);
	if (!ti) {
		LogError("get table(%d) info failed", table_index);
		return false;
	}
	const char* format = config_mgr_.get_field_type_format(table_index, key_name);
	if (!format) {
		LogError("table_index(%d) key_name(%s) get field_type format failed", table_index, key_name);
		return false;
	}

	std::snprintf(buf_[0], sizeof(buf_[0]), "SELECT * FROM %s WHERE %s=%s", ti->name, key_name, format);
	return push_read_cmd(buf_[0], std::strlen(buf_[0]), get_result_func, nullptr, 0);
}

template <typename KeyType, typename KeyType2>
bool MysqlDBManager::selectRecord(
		int table_index,
		const char* key_name, const KeyType& key_value,
		const char* key2_name, const KeyType2& key2_value,
		mysql_cmd_callback_func get_result_func)
{
	const MysqlTableInfo* ti = config_mgr_.get_table_info(table_index);
	if (!ti) {
		LogError("get table(%d) info failed", table_index);
		return false;
	}
	const char* format = config_mgr_.get_field_type_format(table_index, key_name);
	if (!format) {
		LogError("table_index(%d) key_name(%s) get field_type format failed", table_index, key_name);
		return false;
	}

	const char* format2 = config_mgr_.get_field_type_format(table_index, key_name);
	if (!format2) {
		LogError("table_index(%d) key_name(%s) get field_type format failed", table_index, key2_name);
		return false;
	}

	std::snprintf(buf_[0], sizeof(buf_[0]), "SELECT * FROM %s WHERE %s=%s AND %s=%s", ti->name, key_name, format, key2_name, format2);
	return push_read_cmd(buf_[0], std::strlen(buf_[0]), get_result_func, nullptr, 0);
}

template <typename KeyType>
bool MysqlDBManager::selectRecord(
		int table_index,
		const char* key_name, const KeyType& key_value,
		const std::list<const char*>& fields_list,
		mysql_cmd_callback_func get_result_func)
{
	if (fields_list.size() == 0) {
		return selectRecord(table_index, key_name, key_value, get_result_func);
	}

	const MysqlTableInfo* ti = config_mgr_.get_table_info(table_index);
	if (!ti) {
		LogError("get table(%d) info failed", table_index);
		return false;
	}
	const char* format = config_mgr_.get_field_type_format(table_index, key_name);
	if (!format) {
		LogError("table_index(%d) key_name(%s) get field_type format failed", table_index, key_name);
		return false;
	}

	std::snprintf(buf_[0], sizeof(buf_[0]), "SELECT %s FROM %s WHERE %s=%s", boost::join(fields_list, ","), ti->name, key_name, format);
	return push_read_cmd(buf_[0], std::strlen(buf_[0]), get_result_func, nullptr, 0);
}

template <typename KeyType, typename KeyType2>
bool MysqlDBManager::selectRecord(
		int table_index,
		const char* key_name, const KeyType& key_value,
		const char* key2_name, const KeyType2& key2_value,
		const std::list<const char*>& fields_list,
		mysql_cmd_callback_func get_result_func)
{
	if (fields_list.size() == 0) {
		return selectRecord(table_index, key_name, key_value, key2_name, key2_value, get_result_func);
	}

	const MysqlTableInfo* ti = config_mgr_.get_table_info(table_index);
	if (!ti) {
		LogError("get table(%d) info failed", table_index);
		return false;
	}
	const char* format = config_mgr_.get_field_type_format(table_index, key_name);
	if (!format) {
		LogError("table_index(%d) key_name(%s) get field_type format failed", table_index, key_name);
		return false;
	}

	const char* format2 = config_mgr_.get_field_type_format(table_index, key_name);
	if (!format2) {
		LogError("table_index(%d) key_name(%s) get field_type format failed", table_index, key2_name);
		return false;
	}

	std::snprintf(buf_[0], sizeof(buf_[0]), "SELECT %s FROM %s WHERE %s=%s AND %s=%s", boost::join(fields_list, ","), ti->name, key_name, format);
	return push_read_cmd(buf_[0], std::strlen(buf_[0]), get_result_func, nullptr, 0);
}
