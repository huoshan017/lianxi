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
	template <typename FieldNameValueArg>
	char* format_insert_field_name_str(const char* head_buf, int buf_num, const FieldNameValueArg& arg);
	template <typename FirstFieldNameValueArg, typename... RestFieldNameValueArg>
	char* format_insert_field_name_str(const char* head_buf, int buf_num, const FirstFieldNameValueArg& first, const RestFieldNameValueArg&... rest);

	template <typename FieldNameValueArg>
	char* format_insert_field_value_str(int table_index, const char* head_buf, int buf_num, const FieldNameValueArg& arg);
	template <typename FirstFieldNameValueArg, typename... RestFieldNameValueArg>
	char* format_insert_field_value_str(int table_index, const char* head_buf, int buf_num, const FirstFieldNameValueArg& first, const RestFieldNameValueArg&... rest);

	template <typename FieldNameValueArg>
	char* format_update_field_value_str(int table_index, const FieldNameValueArg& arg);
	template <typename FirstFieldNameValueArg, typename... RestFieldNameValueArg>
	char* format_update_field_value_str(int table_index, const FirstFieldNameValueArg& first, const RestFieldNameValueArg&... rest);

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
	char buf_[3][2048];
	char buf2_[3][2048];
	char big_buf_[3][1024*128];
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
		std::snprintf(buf, buf_len, "%s", arg.field_name.c_str());
	else
		std::snprintf(buf, buf_len, "%s, %s", head_buf, arg.field_name.c_str());
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
	const MysqlTableInfo* ti = config_mgr_.get_table_info(table_index);
	if (!ti) {
		LogError("get table(%d) info failed", table_index);
		return nullptr;
	}
	int idx = config_mgr_.get_table_field_index(table_index, arg.field_name.c_str());
	if (idx < 0) {
		LogError("not found field_name(%s)", arg.field_name.c_str());
		return nullptr;
	}

	char* buf = nullptr;
	int buf_len = 0;
	if (!get_buf_info(buf_num, buf, buf_len)) {
		return nullptr;
	}

	int ft = ti->fields_info[idx].field_type;
	int flags = ti->fields_info[idx].create_flags;
	bool b = mysql_get_field_value_format((MysqlTableFieldType)ft, flags, arg.field_value, buf, buf_len);
	if (!b) {
		LogError("field_type(%d), create_flags(%d) get format error", ft, flags);
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
	return format_insert_field_value_str(table_index, head_buf, buf_num, rest...);
}

template <typename FieldNameValueArg>
char* MysqlDBManager::format_update_field_value_str(int table_index, const FieldNameValueArg& arg)
{
	return nullptr;
}

template <typename FirstFieldNameValueArg, typename... RestFieldNameValueArg>
char* MysqlDBManager::format_update_field_value_str(int table_index, const FirstFieldNameValueArg& first, const RestFieldNameValueArg&... rest)
{
	return nullptr;
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
	bool res = push_insert_cmd(big_buf_[big_index_], strlen(big_buf_[big_index_]), get_last_insert_id_func, param, param_l);
	if (!get_last_insert_id_func) return res;
	return push_get_last_insert_id_cmd(get_last_insert_id_func, param, param_l);
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

#if 0
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
#endif
	return false;
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
	const char* format = config_mgr_.get_field_type_format(table_index, key_name, key_value, buf_[index_], sizeof(buf_[index_]));
	if (!format) {
		LogError("table_index(%d) key_name(%s) get field_type format failed", table_index, key_name);
		return false;
	}

	std::snprintf(big_buf_[big_index_], sizeof(big_buf_[big_index_]), "SELECT * FROM %s WHERE %s=%s", ti->name, key_name, format);
	return push_read_cmd(big_buf_[big_index_], std::strlen(big_buf_[big_index_]), get_result_func, nullptr, 0);
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
	const char* format = config_mgr_.get_field_type_format(table_index, key_name, key_value, buf_[index_], sizeof(buf_[index_]));
	if (!format) {
		LogError("table_index(%d) key_name(%s) get field_type format failed", table_index, key_name);
		return false;
	}

	const char* format2 = config_mgr_.get_field_type_format(table_index, key_name, key_value, buf2_[index2_], sizeof(buf2_[index2_]));
	if (!format2) {
		LogError("table_index(%d) key_name(%s) get field_type format failed", table_index, key2_name);
		return false;
	}

	std::snprintf(big_buf_[big_index_], sizeof(big_buf_[big_index_]), "SELECT * FROM %s WHERE %s=%s AND %s=%s", ti->name, key_name, format, key2_name, format2);
	return push_read_cmd(big_buf_[big_index_], std::strlen(big_buf_[big_index_]), get_result_func, nullptr, 0);
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
	const char* format = config_mgr_.get_field_type_format(table_index, key_name, key_value, buf_[index_], sizeof(buf_[index_]));
	if (!format) {
		LogError("table_index(%d) key_name(%s) get field_type format failed", table_index, key_name);
		return false;
	}

	std::snprintf(buf_[index_], sizeof(buf_[index_]),
			"SELECT %s FROM %s WHERE %s=%s", boost::join(fields_list, ","), ti->name, key_name, format);
	return push_read_cmd(buf_[index_], std::strlen(buf_[index_]), get_result_func, nullptr, 0);
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
	const char* format = config_mgr_.get_field_type_format(table_index, key_name, key_value, buf_[index_], sizeof(buf_[index_]));
	if (!format) {
		LogError("table_index(%d) key_name(%s) get field_type format failed", table_index, key_name);
		return false;
	}

	const char* format2 = config_mgr_.get_field_type_format(table_index, key_name, key_value, buf2_[index2_], sizeof(buf2_[index2_]));
	if (!format2) {
		LogError("table_index(%d) key_name(%s) get field_type format failed", table_index, key2_name);
		return false;
	}

	std::snprintf(big_buf_[big_index_], sizeof(big_buf_[big_index_]), "SELECT %s FROM %s WHERE %s=%s AND %s=%s",
			boost::join(fields_list, ","), ti->name, key_name, format);
	return push_read_cmd(big_buf_[big_index_], std::strlen(big_buf_[big_index_]), get_result_func, nullptr, 0);
}
