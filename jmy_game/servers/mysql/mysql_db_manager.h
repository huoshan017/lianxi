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

	bool init(const std::string& host, const std::string& user, const std::string& password, const MysqlDatabaseConfig& config);
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
	template <typename KeyType, typename KeyType2, typename... FieldNameValueArgs>
	bool updateRecord(const char* table_name, const char* key_name, const KeyType& key_value, const char* key2_name, const KeyType2& key2_value, const FieldNameValueArgs&... args);
	template <typename KeyType, typename KeyType2, typename... FieldNameValueArgs>
	bool updateRecord(int table_index, const char* key_name, const KeyType& key_value, const char* key2_name, const KeyType2& key2_value, const FieldNameValueArgs&... args);
	template <typename KeyType, typename KeyType2, typename KeyType3, typename... FieldNameValueArgs>
	bool updateRecord(const char* table_name, const char* key_name, const KeyType& key_value, const char* key2_name, const KeyType2& key2_value, const char* key3_name, const KeyType3& key3_value, const FieldNameValueArgs&... args);
	template <typename KeyType, typename KeyType2, typename KeyType3, typename... FieldNameValueArgs>
	bool updateRecord(int table_index, const char* key_name, const KeyType& key_value, const char* key2_name, const KeyType2& key2_value, const  char* key3_name, const KeyType3& key3_value, const FieldNameValueArgs&... args);

	template <typename KeyType>
	bool deleteRecord(const char* table_name, const char* key_name, const KeyType& key_value);
	template <typename KeyType>
	bool deleteRecord(int table_index, const char* key_name, const KeyType& key_value);


	bool selectRecord(const char* table_name, std::list<const char*>& field_list, mysql_cmd_callback_func get_result_func, void* param, long param_l);
	bool selectRecord(int table_index, std::list<const char*>& field_list, mysql_cmd_callback_func get_result_func, void* param, long param_l);

	template <typename KeyType>
	bool selectRecord(const char* table_name,
			const char* key_name, const KeyType& key_value,
			mysql_cmd_callback_func get_result_func,
			void* param, long param_l);

	template <typename KeyType, typename KeyType2>
	bool selectRecord(const char* table_name, int key_relation,
			const char* key_name, const KeyType& key_value,
			const char* key2_name, const KeyType2& key2_value,
			mysql_cmd_callback_func get_result_func,
			void* param, long param_l);

	template <typename KeyType>
	bool selectRecord(const char* table_name,
			const char* key_name, const KeyType& key_value,
			const char** field_name_array, int field_name_array_length,
			mysql_cmd_callback_func get_result_func,
			void* param, long param_l);

	template <typename KeyType>
	bool selectRecord(const char* table_name,
			const char* key_name, const KeyType& key_value,
			const std::list<const char*>& fields_list,
			mysql_cmd_callback_func get_result_func,
			void* param, long param_l);

	template <typename KeyType, typename KeyType2>
	bool selectRecord(const char* table_name, int key_relation,
			const char* key_name, const KeyType& key_value,
			const char* key2_name, const KeyType2& key2_value,
			const std::list<const char*>& fields_list,
			mysql_cmd_callback_func get_result_func,
			void* param, long param_l);

	template <typename KeyType>
	bool selectRecord(int table_index,
			const char* key_name, const KeyType& key_value,
			mysql_cmd_callback_func get_result_func,
			void* param, long param_l);

	template <typename KeyType, typename KeyType2>
	bool selectRecord(int table_index, int key_relation,
			const char* key_name, const KeyType& key_value,
			const char* key2_name, const KeyType2& key2_value,
			mysql_cmd_callback_func get_result_func,
			void* param, long param_l);

	template <typename KeyType>
	bool selectRecord(int table_index,
			const char* key_name, const KeyType& key_value,
			const std::list<const char*>& fields_list,
			mysql_cmd_callback_func get_result_func,
			void* param, long param_l);

	template <typename KeyType, typename KeyType2>
	bool selectRecord(int table_index, int key_relation,
			const char* key_name, const KeyType& key_value,
			const char* key2_name, const KeyType2& key2_value,
			const std::list<const char*>& fields_list,
			mysql_cmd_callback_func get_result_func,
			void* param, long param_l);

	bool pushSelectCmd(const char* sql, unsigned int sql_len, mysql_cmd_callback_func get_result_func, void* param, long param_l) {
		return push_read_cmd(sql, sql_len, get_result_func, param, param_l);
	}

	bool pushWriteCmd(const char* sql, unsigned int sql_len) {
		return push_write_cmd(sql, sql_len);
	}

private:
	template <typename FieldNameValueArg>
	char* format_insert_field_name_str(const char* head_buf, int buf_num, const FieldNameValueArg& arg);
	template <typename FirstFieldNameValueArg, typename... RestFieldNameValueArg>
	char* format_insert_field_name_str(const char* head_buf, int buf_num, const FirstFieldNameValueArg& first, const RestFieldNameValueArg&... rest);

	template <typename FieldNameValueArg>
	char* format_insert_field_value_str(int table_index, const char* head_buf, int buf_num, const FieldNameValueArg& arg);
	template <typename FirstFieldNameValueArg, typename... RestFieldNameValueArg>
	char* format_insert_field_value_str(int table_index, const char* head_buf, int buf_num, const FirstFieldNameValueArg& first, const RestFieldNameValueArg&... rest);

	template <typename FieldNameValueArg>
	char* format_update_field_value_str(int table_index, const char* head_buf, int buf_num, const FieldNameValueArg& arg);
	template <typename FirstFieldNameValueArg, typename... RestFieldNameValueArg>
	char* format_update_field_value_str(int table_index, const char* head_buf, int buf_num, const FirstFieldNameValueArg& first, const RestFieldNameValueArg&... rest);

	bool push_read_cmd(const char* sql, unsigned int sql_len, mysql_cmd_callback_func get_result_func, void* param, long param_l);
	bool push_write_cmd(const char* sql, unsigned int sql_len);
	bool push_insert_cmd(const char* sql, unsigned int sql_len, mysql_cmd_callback_func get_last_insert_id_func, void* param, long param_l);
	bool push_get_last_insert_id_cmd(mysql_cmd_callback_func get_last_insert_id_func, void* param, long param_l);

	int next_index() {
		index_ += 1;
		if (index_ >= (int)(sizeof(buf_)/sizeof(buf_[0])))
			index_ = 0;
		return index_;
	}

	int next_index2() {
		index2_ += 1;
		if (index2_ >= (int)(sizeof(buf2_)/sizeof(buf2_[0])))
			index2_ = 0;
		return index2_;
	}

	int next_big_index() {
		big_index_ += 1;
		if (big_index_ >= (int)(sizeof(big_buf_)/sizeof(big_buf_[0])))
			big_index_ = 0;
		return big_index_;
	}

	bool get_buf_info(int buf_num, char*& buf, int& buf_len) {
		if (buf_num == 0) {
			buf = buf_[index_];
			buf_len = sizeof(buf_[index_]);
		} else if (buf_num == 1) {
			buf = buf2_[index2_];
			buf_len = sizeof(buf2_[index2_]);
		} else {
			return false;
		}
		return true;
	}

	int to_next_index(int buf_num) {
		if (buf_num == 0) return next_index();
		else if (buf_num == 1) return next_index2();
		else return -1;
	}

private:
	MysqlDBConfigManager config_mgr_;
	MysqlConnectorPool conn_pool_;
	char buf_[3][1024*32];
	char buf2_[3][1024*32];
	char big_buf_[2][1024*128];
	int index_, index2_, big_index_;
};

template <typename FieldNameValueArg>
char* MysqlDBManager::format_insert_field_name_str(const char* head_buf, int buf_num, const FieldNameValueArg& arg)
{
	char* buf = nullptr;
	int buf_len = 0;
	if (!get_buf_info(buf_num, buf, buf_len)) {
		return nullptr;
	}
	if (!head_buf)
		std::snprintf(buf, buf_len, "%s", arg.field_name);
	else
		std::snprintf(buf, buf_len, "%s, %s", head_buf, arg.field_name);
	return buf;
}

template <typename FirstFieldNameValueArg, typename... RestFieldNameValueArg>
char* MysqlDBManager::format_insert_field_name_str(const char* head_buf, int buf_num, const FirstFieldNameValueArg& first, const RestFieldNameValueArg&... rest)
{
	char* t = format_insert_field_name_str(head_buf, buf_num, first);
	if (!t) return nullptr;
	if (to_next_index(buf_num) < 0) return nullptr;
	return format_insert_field_name_str(t, buf_num, rest...);
}

template <typename FieldNameValueArg>
char* MysqlDBManager::format_insert_field_value_str(int table_index, const char* head_buf, int buf_num, const FieldNameValueArg& arg)
{
	char* buf = nullptr;
	int buf_len = 0;
	if (!get_buf_info(buf_num, buf, buf_len)) {
		return nullptr;
	}
	
	const MysqlTableFieldInfo* field_info = config_mgr_.get_field_info(table_index, arg.field_name);
	if (!field_info) {
		return nullptr;
	}

	if (!mysql_get_field_value_format((MysqlTableFieldType)field_info->field_type, field_info->create_flags, arg.field_value, buf, buf_len)) {
		LogError("field_type(%d), create_flags(%d) get format error", field_info->field_type, field_info->create_flags);
		return nullptr;
	}

	if (head_buf) {
		char* prev_buf = buf;
		if (to_next_index(buf_num) < 0) return nullptr;
		if (!get_buf_info(buf_num, buf, buf_len)) {
			return nullptr;
		}
		std::snprintf(buf, buf_len, "%s, %s", head_buf, prev_buf);
	}
	return buf;
}

template <typename FirstFieldNameValueArg, typename... RestFieldNameValueArg>
char* MysqlDBManager::format_insert_field_value_str(int table_index, const char* head_buf, int buf_num, const FirstFieldNameValueArg& first, const RestFieldNameValueArg&... rest)
{
	char* t = format_insert_field_value_str(table_index, head_buf, buf_num, first);
	if (!t) return nullptr;
	if (to_next_index(buf_num) < 0) return nullptr;
	return format_insert_field_value_str(table_index, t, buf_num, rest...);
}

template <typename FieldNameValueArg>
char* MysqlDBManager::format_update_field_value_str(int table_index, const char* head_buf, int buf_num, const FieldNameValueArg& arg)
{
	if (!arg.field_name)
		return (char*)head_buf;

	const MysqlTableFieldInfo* field_info = config_mgr_.get_field_info(table_index, arg.field_name);
	if (!field_info) {
		return nullptr;
	}

	char tmp[32];
	if (!mysql_get_field_value_format((MysqlTableFieldType)field_info->field_type, field_info->create_flags, arg.field_value, tmp, sizeof(tmp))) {
		LogError("field_type(%d), create_flags(%d) get format error", field_info->field_type, field_info->create_flags);
		return nullptr;
	}

	char* buf = nullptr; int buf_len = 0;
	if (!get_buf_info(buf_num, buf, buf_len)) {
		return nullptr;
	}
	std::snprintf(buf, buf_len, "%s=%s", arg.field_name, tmp);

	if (head_buf) {
		char* prev_buf = buf;
		if (to_next_index(buf_num) < 0) return nullptr;
		if (!get_buf_info(buf_num, buf, buf_len)) {
			return nullptr;
		}
		std::snprintf(buf, buf_len, "%s, %s", head_buf, prev_buf);
	}
	return buf;
}

template <typename FirstFieldNameValueArg, typename... RestFieldNameValueArg>
char* MysqlDBManager::format_update_field_value_str(int table_index, const char* head_buf, int buf_num, const FirstFieldNameValueArg& first, const RestFieldNameValueArg&... rest)
{
	char* t = format_update_field_value_str(table_index, head_buf, buf_num, first);
	if (to_next_index(buf_num) < 0) return nullptr;
	return format_update_field_value_str(table_index, t, buf_num, rest...);
}

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

	char* field_names_str = format_insert_field_name_str(nullptr, 0, args...);
	char* field_values_str = format_insert_field_value_str(table_index, nullptr, 1, args...);

	std::snprintf(big_buf_[big_index_], sizeof(big_buf_[big_index_]), "INSERT INTO %s (%s) VALUES (%s)", ti->name, field_names_str, field_values_str);
	LogInfo("insert sql string: %s", big_buf_[big_index_]);
	return push_insert_cmd(big_buf_[big_index_], strlen(big_buf_[big_index_]), get_last_insert_id_func, param, param_l);
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

	const char* format = format_update_field_value_str(table_index, nullptr, 0, args...);
	if (!format) return false;
	char tmp[64];
	const char* key_format = config_mgr_.get_field_type_format(table_index, key_name, key_value, tmp, sizeof(tmp));
	if (!key_format) return false;
	std::snprintf(big_buf_[big_index_], sizeof(big_buf_[big_index_]), "UPDATE %s SET %s WHERE %s=%s", ti->name, format, key_name, key_format);
	return push_write_cmd(big_buf_[big_index_], strlen(big_buf_[big_index_]));
}

template <typename KeyType, typename KeyType2, typename... FieldNameValueArgs>
bool MysqlDBManager::updateRecord(const char* table_name, const char* key_name, const KeyType& key_value, const char* key2_name, const KeyType2& key2_value, const FieldNameValueArgs&... args)
{
	int idx = config_mgr_.get_table_index(table_name);
	if (idx < 0) {
		LogError("get table(%s) index failed", table_name);
		return false;
	}
	return updateRecord(idx, key_name, key_value, key2_name, key2_value, args...);
}

template <typename KeyType, typename KeyType2, typename... FieldNameValueArgs>
bool MysqlDBManager::updateRecord(int table_index, const char* key_name, const KeyType& key_value, const char* key2_name, const KeyType2& key2_value, const FieldNameValueArgs&... args)
{
	const MysqlTableInfo* ti = config_mgr_.get_table_info(table_index);
	if (!ti) {
		LogError("get table(%d) info failed", table_index);
		return false;
	}

	const char* format = format_update_field_value_str(table_index, nullptr, 0, args...);
	if (!format) return false;
	char tmp[64], tmp2[64];
	const char* key_format = config_mgr_.get_field_type_format(table_index, key_name, key_value, tmp, sizeof(tmp));
	if (!key_format) return false;
	const char* key2_format = config_mgr_.get_field_type_format(table_index, key2_name, key2_value, tmp2, sizeof(tmp2));
	if (!key2_format) return false;
	std::snprintf(big_buf_[big_index_], sizeof(big_buf_[big_index_]), "UPDATE %s SET %s WHERE %s=%s AND %s=%s",
			ti->name, format, key_name, key_format, key2_name, key2_format);
	return push_write_cmd(big_buf_[big_index_], strlen(big_buf_[big_index_]));
}

template <typename KeyType, typename KeyType2, typename KeyType3, typename... FieldNameValueArgs>
bool MysqlDBManager::updateRecord(const char* table_name, const char* key_name, const KeyType& key_value, const char* key2_name, const KeyType2& key2_value, const char* key3_name, const KeyType3& key3_value, const FieldNameValueArgs&... args)
{
	int idx = config_mgr_.get_table_index(table_name);
	if (idx < 0) {
		LogError("get table(%s) index failed", table_name);
		return false;
	}
	return updateRecord(idx, key_name, key_value, key2_name, key2_value, key3_name, key3_value, args...);
}

template <typename KeyType, typename KeyType2, typename KeyType3, typename... FieldNameValueArgs>
bool MysqlDBManager::updateRecord(int table_index, const char* key_name, const KeyType& key_value, const char* key2_name, const KeyType2& key2_value, const  char* key3_name, const KeyType3& key3_value, const FieldNameValueArgs&... args)
{
	const MysqlTableInfo* ti = config_mgr_.get_table_info(table_index);
	if (!ti) {
		LogError("get table(%d) info failed", table_index);
		return false;
	}

	const char* format = format_update_field_value_str(table_index, nullptr, 0, args...);
	if (!format) return false;
	char tmp[64], tmp2[64], tmp3[64];
	const char* key_format = config_mgr_.get_field_type_format(table_index, key_name, key_value, tmp, sizeof(tmp));
	if (!key_format) return false;
	const char* key2_format = config_mgr_.get_field_type_format(table_index, key2_name, key2_value, tmp2, sizeof(tmp2));
	if (!key2_format) return false;
	const char* key3_format = config_mgr_.get_field_type_format(table_index, key3_name, key3_value, tmp3, sizeof(tmp3));
	if (!key3_format) return false;

	std::snprintf(big_buf_[big_index_], sizeof(big_buf_[big_index_]), "UPDATE %s SET %s WHERE %s=%s AND %s=%s AND %s=%s",
			ti->name, format, key_name, key_format, key2_name, key2_format, key3_name, key3_format);
	return push_write_cmd(big_buf_[big_index_], strlen(big_buf_[big_index_]));
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
	const char* format = config_mgr_.get_field_type_format(table_index, key_name, key_value, buf_[index_], sizeof(buf_[index_]));
	if (!format) {
		LogError("table_index(%d) key_name(%s) get field_type format failed", table_index, key_name);
		return false;
	}
	std::snprintf(big_buf_[big_index_], sizeof(big_buf_[big_index_]), "DELETE FROM %s WHERE %s=%s", ti->name, key_name, format);
	return push_write_cmd(big_buf_[big_index_], std::strlen(big_buf_[big_index_]));
}

template <typename KeyType>
bool MysqlDBManager::selectRecord(
		const char* table_name,
		const char* key_name, const KeyType& key_value,
		mysql_cmd_callback_func get_result_func,
		void* param, long param_l)
{
	int idx = config_mgr_.get_table_index(table_name);
	if (idx < 0) {
		LogError("get table(%s) index failed", table_name);
		return false;
	}
	return selectRecord(idx, key_name, key_value, get_result_func, param, param_l);
}

template <typename KeyType, typename KeyType2>
bool MysqlDBManager::selectRecord(
		const char* table_name, int key_relation,
		const char* key_name, const KeyType& key_value,
		const char* key2_name, const KeyType2& key2_value,
		mysql_cmd_callback_func get_result_func,
		void* param, long param_l)
{
	int idx = config_mgr_.get_table_index(table_name);
	if (idx < 0) {
		LogError("get table(%s) index failed", table_name);
		return false;
	}
	return selectRecord(idx, key_relation, key_name, key_value, key2_name, key2_value, get_result_func, param, param_l);
}

template <typename KeyType>
bool MysqlDBManager::selectRecord(const char* table_name,
			const char* key_name, const KeyType& key_value,
			const char** field_name_array, int field_name_array_length,
			mysql_cmd_callback_func get_result_func,
			void* param, long param_l)
{
	int table_index = config_mgr_.get_table_index(table_name);
	if (table_index < 0) {
		LogError("get table(%s) index failed", table_name);
		return false;
	}

	if (field_name_array_length == 0) {
		return selectRecord(table_index, key_name, key_value, get_result_func, param, param_l);
	}

	const MysqlTableInfo* ti = config_mgr_.get_table_info(table_index);
	if (!ti) {
		LogError("get table(%d) info failed", table_index);
		return false;
	}

	char* buf = nullptr; int buf_len = 0;
	if (!get_buf_info(0, buf, buf_len)) {
		return false;
	}

	const char* key_value_format = config_mgr_.get_field_type_format(table_index, key_name, key_value, buf, buf_len);
	if (!key_value_format) {
		LogError("table_index(%d) key_name(%s) get field_type format failed", table_index, key_name);
		return false;
	}

	char* fields_name_format = nullptr; 
	char* prev_buf = nullptr;
	for (int i=0; i<field_name_array_length; ++i) {
		if (!get_buf_info(1, fields_name_format, buf_len)) {
			return false;
		}
		if (!prev_buf)
			std::snprintf(fields_name_format, buf_len, "%s", field_name_array[i]);
		else
			std::snprintf(fields_name_format, buf_len, "%s, %s", prev_buf, field_name_array[i]);

		prev_buf = fields_name_format;
		if (to_next_index(1) < 0)
			return false;
	}

	std::snprintf(big_buf_[big_index_], sizeof(big_buf_[big_index_]),
			"SELECT %s FROM %s WHERE %s=%s", fields_name_format, ti->name, key_name, key_value_format);
	return push_read_cmd(big_buf_[big_index_], std::strlen(big_buf_[big_index_]), get_result_func, param, param_l);
}

template <typename KeyType>
bool MysqlDBManager::selectRecord(
		const char* table_name,
		const char* key_name, const KeyType& key_value,
		const std::list<const char*>& fields_list,
		mysql_cmd_callback_func get_result_func,
		void* param, long param_l)
{
	int idx = config_mgr_.get_table_index(table_name);
	if (idx < 0) {
		LogError("get table(%s) index failed", table_name);
		return false;
	}
	return selectRecord(idx, key_name, key_value, fields_list, get_result_func, param, param_l);
}

template <typename KeyType, typename KeyType2>
bool MysqlDBManager::selectRecord(
		const char* table_name, int key_relation,
		const char* key_name, const KeyType& key_value,
		const char* key2_name, const KeyType2& key2_value,
		const std::list<const char*>& fields_list,
		mysql_cmd_callback_func get_result_func,
		void* param, long param_l)
{
	int idx = config_mgr_.get_table_index(table_name);
	if (idx < 0) {
		LogError("get table(%s) index failed", table_name);
		return false;
	}
	return selectRecord(idx, key_relation, key_name, key_value, key2_name, key2_value, fields_list, get_result_func, param, param_l);
}

template <typename KeyType>
bool MysqlDBManager::selectRecord(
		int table_index,
		const char* key_name, const KeyType& key_value,
		mysql_cmd_callback_func get_result_func,
		void* param, long param_l)
{
	const MysqlTableInfo* ti = config_mgr_.get_table_info(table_index);
	if (!ti) {
		LogError("get table(%d) info failed", table_index);
		return false;
	}
	const char* format = config_mgr_.get_field_type_format(table_index, key_name, key_value, buf_[index_], sizeof(buf_[index_]));
	if (!format) {
		LogError("table_index(%d) key_name(%s) get field_type format failed", table_index, key_name);
		return false;
	}

	std::snprintf(big_buf_[big_index_], sizeof(big_buf_[big_index_]), "SELECT * FROM %s WHERE %s=%s", ti->name, key_name, format);
	return push_read_cmd(big_buf_[big_index_], std::strlen(big_buf_[big_index_]), get_result_func, param, param_l);
}

template <typename KeyType, typename KeyType2>
bool MysqlDBManager::selectRecord(
		int table_index, int key_relation,
		const char* key_name, const KeyType& key_value,
		const char* key2_name, const KeyType2& key2_value,
		mysql_cmd_callback_func get_result_func,
		void* param, long param_l)
{
	const MysqlTableInfo* ti = config_mgr_.get_table_info(table_index);
	if (!ti) {
		LogError("get table(%d) info failed", table_index);
		return false;
	}
	const char* format = config_mgr_.get_field_type_format(table_index, key_name, key_value, buf_[index_], sizeof(buf_[index_]));
	if (!format) {
		LogError("table_index(%d) key_name(%s) get field_type format failed", table_index, key_name);
		return false;
	}

	const char* format2 = config_mgr_.get_field_type_format(table_index, key2_name, key2_value, buf2_[index2_], sizeof(buf2_[index2_]));
	if (!format2) {
		LogError("table_index(%d) key_name(%s) get field_type format failed", table_index, key2_name);
		return false;
	}

	const char* key_rela_str = key_relation==0 ? "AND" : "OR";
	std::snprintf(big_buf_[big_index_], sizeof(big_buf_[big_index_]),
			"SELECT * FROM %s WHERE %s=%s %s %s=%s", ti->name, key_name, format, key_rela_str, key2_name, format2);
	return push_read_cmd(big_buf_[big_index_], std::strlen(big_buf_[big_index_]), get_result_func, param, param_l);
}

template <typename KeyType>
bool MysqlDBManager::selectRecord(
		int table_index,
		const char* key_name, const KeyType& key_value,
		const std::list<const char*>& fields_list,
		mysql_cmd_callback_func get_result_func,
		void* param, long param_l)
{
	if (fields_list.size() == 0) {
		return selectRecord(table_index, key_name, key_value, get_result_func, param, param_l);
	}

	const MysqlTableInfo* ti = config_mgr_.get_table_info(table_index);
	if (!ti) {
		LogError("get table(%d) info failed", table_index);
		return false;
	}

	char* buf = nullptr;
	int buf_len = 0;
	if (!get_buf_info(0, buf, buf_len)) {
		return false;
	}

	const char* format = config_mgr_.get_field_type_format(table_index, key_name, key_value, buf, buf_len);
	if (!format) {
		LogError("table_index(%d) key_name(%s) get field_type format failed", table_index, key_name);
		return false;
	}

	std::snprintf(big_buf_[big_index_], sizeof(big_buf_[big_index_]), "SELECT %s FROM %s WHERE %s=%s", boost::join(fields_list, ","), ti->name, key_name, format);
	return push_read_cmd(big_buf_[big_index_], std::strlen(big_buf_[big_index_]), get_result_func, param, param_l);
}

template <typename KeyType, typename KeyType2>
bool MysqlDBManager::selectRecord(
		int table_index, int key_relation,
		const char* key_name, const KeyType& key_value,
		const char* key2_name, const KeyType2& key2_value,
		const std::list<const char*>& fields_list,
		mysql_cmd_callback_func get_result_func,
		void* param, long param_l)
{
	if (fields_list.size() == 0) {
		return selectRecord(table_index, key_relation, key_name, key_value, key2_name, key2_value, get_result_func, param, param_l);
	}

	const MysqlTableInfo* ti = config_mgr_.get_table_info(table_index);
	if (!ti) {
		LogError("get table(%d) info failed", table_index);
		return false;
	}
	const char* format = config_mgr_.get_field_type_format(table_index, key_name, key_value, buf_[index_], sizeof(buf_[index_]));
	if (!format) {
		LogError("table_index(%d) key_name(%s) get field_type format failed", table_index, key_name);
		return false;
	}

	const char* format2 = config_mgr_.get_field_type_format(table_index, key2_name, key2_value, buf2_[index2_], sizeof(buf2_[index2_]));
	if (!format2) {
		LogError("table_index(%d) key_name(%s) get field_type format failed", table_index, key2_name);
		return false;
	}

	const char* key_rela_str = key_relation==0 ? "AND" : "OR";
	std::snprintf(big_buf_[big_index_], sizeof(big_buf_[big_index_]), "SELECT %s FROM %s WHERE %s=%s %s %s=%s",
			boost::join(fields_list, ","), ti->name, key_name, format, key_rela_str, key2_name, format2);
	return push_read_cmd(big_buf_[big_index_], std::strlen(big_buf_[big_index_]), get_result_func, param, param_l);
}
