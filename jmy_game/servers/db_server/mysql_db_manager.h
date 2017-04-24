#pragma once

#include "mysql_connector_pool.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <tuple>
#include "../common/util.h"
#include "mysql_util.h"

class MysqlDBManager
{
public:
	MysqlDBManager();
	~MysqlDBManager();

	bool init(const MysqlDatabaseConfig& config);
	void clear();
	int run();

	bool insertRecord(const char* table_name, mysql_cmd_callback_func get_last_insert_id_func);
	template <typename... FieldNameValueArgs>
	bool insertRecord(const char* table_name, const FieldNameValueArgs&... args);
	template <typename... FieldNameValueArgs>
	bool insertRecord(const char* table_name, mysql_cmd_callback_func get_last_insert_id_func, const FieldNameValueArgs&... args);

	bool insertRecord(int table_index, mysql_cmd_callback_func get_last_insert_id_func);
	template <typename... FieldNameValueArgs>
	bool insertRecord(int table_index, const FieldNameValueArgs&... args);
	template <typename... FieldNameValueArgs>
	bool insertRecord(int table_index, mysql_cmd_callback_func get_last_insert_id_func, const FieldNameValueArgs&... args);

	template <typename KeyType, typename... FieldNameValueArgs>
	bool updateRecord(const char* table_name, const char* key_name, const KeyType& key_value, const FieldNameValueArgs&... args);
	template <typename KeyType, typename... FieldNameValueArgs>
	bool updateRecord(int table_index, const char* key_name, const KeyType& key_value, const FieldNameValueArgs&... args);

	template <typename KeyType>
	bool deleteRecord(const char* table_name, const char* key_name, const KeyType& key_value);
	template <typename KeyType>
	bool deleteRecord(int table_index, const char* key_name, const KeyType& key_value);

	template <typename KeyType>
	bool selectRecord(const char* table_name, const char* key_name, const KeyType& key_value, mysql_cmd_callback_func get_result_func);
	template <typename KeyType, typename KeyType2>
	bool selectRecord(const char* table_name, const char* key_name, const KeyType& key_value, const char* key2_name, const KeyType2& key2_value, mysql_cmd_callback_func get_result_func);
	template <typename KeyType>
	bool selectRecord(int table_index, const char* key_name, const KeyType& key_value, mysql_cmd_callback_func get_result_func);
	template <typename KeyType, typename KeyType2>
	bool selectRecord(int table_index, const char* key_name, const KeyType& key_value, const char* key2_name, const KeyType2& key2_value, mysql_cmd_callback_func get_result_func);

private:
	bool push_read_cmd(const char* sql, unsigned int sql_len, mysql_cmd_callback_func get_result_func, void* param, long param_l);
	bool push_write_cmd(const char* sql, unsigned int sql_len, void* param, long param_l);
	bool push_write_cmd(const char* sql, unsigned int sql_len, mysql_cmd_callback_func get_last_insert_id_func, void* param, long param_l);
	int get_field_type(int table_index, const char* field_name, const char** table_name);
	
private:
	std::string db_name_;
	std::unordered_map<std::string, int> table_name2index_;
	std::vector<MysqlTableInfo*> table_array_;
	typedef std::unordered_map<std::string, int> table_fields_name2index_type;
	std::vector<table_fields_name2index_type> table_fields_name2index_array_;
	MysqlConnectorPool conn_pool_;
	char buf_[2][1024*128];
	static const char* s_last_insert_id_str_;
};

template <typename... FieldNameValueArgs>
bool MysqlDBManager::insertRecord(const char* table_name, const FieldNameValueArgs&... args)
{
	std::unordered_map<std::string, int>::iterator it = table_name2index_.find(std::string(table_name));
	if (it == table_name2index_.end()) {
		return false;
	}
	int idx = it->second;
	return insertRecord(idx, args...);
}

template <typename... FieldNameValueArgs>
bool MysqlDBManager::insertRecord(const char* table_name, mysql_cmd_callback_func get_last_insert_id_func, const FieldNameValueArgs&... args)
{
	std::unordered_map<std::string, int>::iterator it = table_name2index_.find(std::string(table_name));
	if (it == table_name2index_.end())
		return false;
	int idx = it->second;
	return insertRecord(idx, get_last_insert_id_func, args...);
}

template <typename... FieldNameValueArgs>
bool MysqlDBManager::insertRecord(int table_index, const FieldNameValueArgs&... args)
{
	return insertRecord(table_index, nullptr, args...);
}

template <typename... FieldNameValueArgs>
bool MysqlDBManager::insertRecord(int table_index, mysql_cmd_callback_func get_last_insert_id_func, const FieldNameValueArgs&... args)
{
	if (table_index<0 || table_index>=table_array_.size() || table_index>=table_fields_name2index_array_.size())
		return false;
	
	MysqlTableInfo* ti = table_array_[table_index];
	if (!ti)
		return false;

	table_fields_name2index_type& tf = table_fields_name2index_array_[table_index];

	std::tuple<FieldNameValueArgs...> t(args...);
	size_t s = sizeof...(FieldNameValueArgs);
	size_t i = 0;
	table_fields_name2index_type::iterator it;

	std::string types_str;
	char* tmp_values_str = nullptr;
	int bi = 0; int bi_max = 1;
	for (; i<s; ++i) {
		auto v = std::get<i>(t);
		it = tf.find(v.field_name);
		if (it == tf.end()) {
			LogError("not found field_name(%s)", v.field_name);
			return false;
		}

		int idx = it->second;

		// field name
		const char* fn = ti->fields_info[idx].name;
		if (i == 0) {
			types_str = fn;
		} else {
			types_str += (std::string(", ") + fn);
		}

		// field type and value
		int ft = ti->fields_info[idx].field_type;
		if (ft == MYSQL_FIELD_TYPE_TINYINT || ft == MYSQL_FIELD_TYPE_SMALLINT ||
			ft == MYSQL_FIELD_TYPE_MEDIUMINT || ft == MYSQL_FIELD_TYPE_INT) {
			if (!tmp_values_str)
				std::snprintf(buf_[bi], sizeof(buf_[bi]), "%d", v.field_value);
			else {
				std::snprintf(buf_[bi], sizeof(buf_[bi]), "%s, %d", tmp_values_str, v.field_value);
				tmp_values_str = buf_[bi];
			}
		} else if (ft == MYSQL_FIELD_TYPE_BIGINT) {
			if (!tmp_values_str)
				std::snprintf(buf_[bi], sizeof(buf_[bi]), "%lld", v.field_value);
			else {
				std::snprintf(buf_[bi], sizeof(buf_[bi]), "%s, %lld", tmp_values_str, v.field_value);
				tmp_values_str = buf_[bi];
			}
		} else if (IS_MYSQL_TEXT_TYPE(ft)) {
			if (!tmp_values_str)
				std::snprintf(buf_[bi], sizeof(buf_[bi]), "%s", v.field_value);
			else {
				std::snprintf(buf_[bi], sizeof(buf_[bi]), "%s, %s", tmp_values_str, v.field_value);
				tmp_values_str = buf_[bi];
			}
		} else {
			LogWarn("not support type(%d)", ft);
			return false;
		}
		if (i == s-1) {
			tmp_values_str = buf_[bi];
		}
		bi += 1;
		if (bi > bi_max) bi = 0;
	}

	int bi2 = bi + 1;
	if (bi2 > bi_max) {
		bi2 = 0;
	}
	std::snprintf(buf_[bi2], sizeof(buf_[bi2]), "INSERT INTO %s (%s) VALUES (%s)", ti->name, types_str.c_str(), tmp_values_str);
	return push_write_cmd(buf_[bi2], strlen(buf_[bi2]), get_last_insert_id_func, nullptr, 0);
}

template <typename KeyType, typename... FieldNameValueArgs>
bool MysqlDBManager::updateRecord(const char* table_name, const char* key_name, const KeyType& key_value, const FieldNameValueArgs&... args)
{
	std::unordered_map<std::string, int>::iterator it = table_name2index_.find(std::string(table_name));
	if (it == table_name2index_.end())
		return false;
	int idx = it->second;
	return updateRecord(idx, key_name, key_value, args...);
}

template <typename KeyType, typename... FieldNameValueArgs>
bool MysqlDBManager::updateRecord(int table_index, const char* key_name, const KeyType& key_value, const FieldNameValueArgs&... args)
{
	if (table_index<0 || table_index>=table_array_.size() || table_index>=table_fields_name2index_array_.size())
		return false;
	
	MysqlTableInfo* ti = table_array_[table_index];
	if (!ti)
		return false;

	table_fields_name2index_type& tf = table_fields_name2index_array_[table_index];

	std::tuple<FieldNameValueArgs...> t(args...);
	size_t s = sizeof...(FieldNameValueArgs);
	size_t i = 0;
	table_fields_name2index_type::iterator it;

	char* tmp_values_str = nullptr;
	int bi = 0; int bi_max = 1;
	for (; i<s; ++i) {
		auto v = std::get<i>(t);
		it = tf.find(v.field_name);
		if (it == tf.end()) {
			LogError("not found field_name(%s)", v.field_name);
			return false;
		}

		int idx = it->second;

		// field type and value
		int ft = ti->fields_info[idx].field_type;
		if (ft == MYSQL_FIELD_TYPE_TINYINT || ft == MYSQL_FIELD_TYPE_SMALLINT ||
			ft == MYSQL_FIELD_TYPE_MEDIUMINT || ft == MYSQL_FIELD_TYPE_INT) {
			if (!tmp_values_str)
				std::snprintf(buf_[bi], sizeof(buf_[bi]), "%s=%d", v.field_name, v.field_value);
			else {
				std::snprintf(buf_[bi], sizeof(buf_[bi]), "%s, %s=%d", tmp_values_str, v.field_name, v.field_value);
				tmp_values_str = buf_[bi];
			}
		} else if (ft == MYSQL_FIELD_TYPE_BIGINT) {
			if (!tmp_values_str)
				std::snprintf(buf_[bi], sizeof(buf_[bi]), "%s=%lld", v.field_name, v.field_value);
			else {
				std::snprintf(buf_[bi], sizeof(buf_[bi]), "%s, %s=%lld", tmp_values_str, v.field_name, v.field_value);
				tmp_values_str = buf_[bi];
			}
		} else if (IS_MYSQL_TEXT_TYPE(ft)) {
			if (!tmp_values_str)
				std::snprintf(buf_[bi], sizeof(buf_[bi]), "%s=%s", v.field_name, v.field_value);
			else {
				std::snprintf(buf_[bi], sizeof(buf_[bi]), "%s, %s=%s", tmp_values_str, v.field_name, v.field_value);
				tmp_values_str = buf_[bi];
			}
		} else {
			LogWarn("not support type(%d)", ft);
			return false;
		}
		if (i == s-1) {
			tmp_values_str = buf_[bi];
		}
		bi += 1;
		if (bi > bi_max) bi = 0;
	}

	int bi2 = bi + 1;
	if (bi2 > bi_max) {
		bi2 = 0;
	}

	int key_ft = get_field_type(table_index, key_name, nullptr);
	if (key_ft == MYSQL_FIELD_TYPE_TINYINT || key_ft == MYSQL_FIELD_TYPE_SMALLINT ||
		key_ft == MYSQL_FIELD_TYPE_MEDIUMINT || key_ft == MYSQL_FIELD_TYPE_BIGINT) {
		std::snprintf(buf_[bi2], sizeof(buf_[bi2]), "UPDATE_%s SET %s WHERE %s=%d", ti->name, tmp_values_str, key_name, key_value);
	} else if (key_ft == MYSQL_FIELD_TYPE_BIGINT) {
		std::snprintf(buf_[bi2], sizeof(buf_[bi2]), "UPDATE %s SET %s WHERE %s=%lld", ti->name, tmp_values_str, key_name, key_value);
	} else if (IS_MYSQL_TEXT_TYPE(key_ft)) {
		std::snprintf(buf_[bi2], sizeof(buf_[bi2]), "UPDATE %s SET %s WHERE %s=%s", ti->name, tmp_values_str, key_name, key_value);
	} else {
		LogError("not support UPDATE key type(%d)", key_ft);
		return false;
	}
	return push_write_cmd(buf_[bi2], strlen(buf_[bi2]), nullptr, 0);
}

template <typename KeyType>
bool MysqlDBManager::deleteRecord(const char* table_name, const char* key_name, const KeyType& key_value)
{
	std::unordered_map<std::string, int>::iterator it = table_name2index_.find(std::string(table_name));
	if (it == table_name2index_.end())
		return false;
	int idx = it->second;
	return deleteRecord(idx, key_name, key_value);
}

template <typename KeyType>
bool MysqlDBManager::deleteRecord(int table_index, const char* key_name, const KeyType& key_value)
{
	char* table_name = nullptr;
	int ft = get_field_type(table_index, key_name, (const char**)&table_name);
	if (ft == MYSQL_FIELD_TYPE_TINYINT || ft == MYSQL_FIELD_TYPE_SMALLINT ||
		ft == MYSQL_FIELD_TYPE_MEDIUMINT || ft == MYSQL_FIELD_TYPE_INT) {
		std::snprintf(buf_[0], sizeof(buf_[0]), "DELETE FROM %s WHERE %s=%d", table_name, key_name, key_value);
	} else if (ft == MYSQL_FIELD_TYPE_BIGINT) {
		std::snprintf(buf_[0], sizeof(buf_[0]), "DELETE FROM %s WHERE %s=%lld", table_name, key_name, key_value);
	} else if (IS_MYSQL_TEXT_TYPE(ft)){
		std::snprintf(buf_[0], sizeof(buf_[0]), "DELETE FROM %s WHERE %s=%s", table_name, key_name, key_value);
	} else {
		LogWarn("unsupport field value type");
		return false;
	}
	return push_write_cmd(buf_[0], std::strlen(buf_[0]), nullptr, 0);
}

template <typename KeyType>
bool MysqlDBManager::selectRecord(const char* table_name, const char* key_name, const KeyType& key_value, mysql_cmd_callback_func get_result_func)
{
	std::unordered_map<std::string, int>::iterator it = table_name2index_.find(std::string(table_name));
	if (it == table_name2index_.end())
		return false;
	int idx = it->second;
	return selectRecord(idx, key_name, key_value);
}

template <typename KeyType, typename KeyType2>
bool MysqlDBManager::selectRecord(const char* table_name, const char* key_name, const KeyType& key_value, const char* key2_name, const KeyType2& key2_value, mysql_cmd_callback_func get_result_func)
{
	std::unordered_map<std::string, int>::iterator it = table_name2index_.find(std::string(table_name));
	if (it == table_name2index_.end())
		return false;
	return selectRecord(it->second, key_name, key_value, key2_name, key2_value, get_result_func);
}

template <typename KeyType>
bool MysqlDBManager::selectRecord(int table_index, const char* key_name, const KeyType& key_value, mysql_cmd_callback_func get_result_func)
{
	char* table_name = nullptr;
	int ft = get_field_type(table_index, key_name, (const char**)&table_name);
	if (ft == MYSQL_FIELD_TYPE_TINYINT || ft == MYSQL_FIELD_TYPE_SMALLINT ||
		ft == MYSQL_FIELD_TYPE_MEDIUMINT || ft == MYSQL_FIELD_TYPE_INT) {
		std::snprintf(buf_[0], sizeof(buf_[0]), "SELECT * FROM %s WHERE %s=%d", table_name, key_name, key_value);
	} else if (ft == MYSQL_FIELD_TYPE_BIGINT) {
		std::snprintf(buf_[0], sizeof(buf_[0]), "SELECT * FROM %s WHERE %s=%lld", table_name, key_name, key_value);
	} else if (IS_MYSQL_TEXT_TYPE(ft)){
		std::snprintf(buf_[0], sizeof(buf_[0]), "SELECT * FROM %s WHERE %s=%s", table_name, key_name, key_value);
	} else {
		LogWarn("unsupport field value type");
		return false;
	}
	return push_read_cmd(buf_[0], std::strlen(buf_[0]), get_result_func, nullptr, 0);
}

template <typename KeyType, typename KeyType2>
bool MysqlDBManager::selectRecord(int table_index, const char* key_name, const KeyType& key_value, const char* key2_name, const KeyType2& key2_value, mysql_cmd_callback_func get_result_func)
{
	char tmp[128], tmp2[128];
	char* table_name = nullptr;
	int ft = get_field_type(table_index, key_name, (const char**)&table_name);
	if (ft == MYSQL_FIELD_TYPE_TINYINT || ft == MYSQL_FIELD_TYPE_SMALLINT ||
		ft == MYSQL_FIELD_TYPE_MEDIUMINT || ft == MYSQL_FIELD_TYPE_INT) {
		std::snprintf(tmp, sizeof(tmp), "%s=%d", key_name, key_value);
	} else if (ft == MYSQL_FIELD_TYPE_BIGINT) {
		std::snprintf(tmp, sizeof(tmp), "%s=%lld", key_name, key_value);
	} else if (IS_MYSQL_TEXT_TYPE(ft)){
		std::snprintf(tmp, sizeof(tmp), "%s=%s", key_name, key_value);
	} else {
		LogWarn("unsupport field value type");
		return false;
	}

	int ft2 = get_field_type(table_index, key2_name, nullptr);
	if (ft2 == MYSQL_FIELD_TYPE_TINYINT || ft2 == MYSQL_FIELD_TYPE_SMALLINT ||
		ft2 == MYSQL_FIELD_TYPE_MEDIUMINT || ft2 == MYSQL_FIELD_TYPE_INT) {
		std::snprintf(tmp2, sizeof(tmp2), "%s=%d", key2_name, key2_value);
	} else if (ft == MYSQL_FIELD_TYPE_BIGINT) {
		std::snprintf(tmp2, sizeof(tmp2), "%s=%lld", key2_name, key2_value);
	} else if (IS_MYSQL_TEXT_TYPE(ft)) {
		std::snprintf(tmp2, sizeof(tmp2), "%s=%s", key2_name, key2_value);
	} else {
		LogWarn("unsupport field value type");
		return false;
	}

	std::snprintf(buf_[0], sizeof(buf_[0]), "SELECT * FROM %s WHERE %s AND %s", table_name, tmp, tmp2);
	return push_read_cmd(buf_[0], std::strlen(buf_[0]), get_result_func, nullptr, 0);
}
