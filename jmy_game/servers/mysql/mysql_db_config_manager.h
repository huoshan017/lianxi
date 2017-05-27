#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include "mysql_defines.h"

struct MysqlDatabaseConfig;
struct MysqlTableInfo;
struct MysqlTableFieldInfo;
class MysqlConnector;
class MysqlDBConfigManager
{
public:
	MysqlDBConfigManager();
	~MysqlDBConfigManager();

	bool init(const MysqlDatabaseConfig& config);
	void clear();

	int get_table_index(const char* table_name);
	int get_table_field_index(int table_index, const char* field_name);
	const MysqlTableInfo* get_table_info(int table_index);
	const MysqlTableFieldInfo* get_field_info(int table_index, const char* field_name);
	template <typename FieldType>
	const char* get_field_type_format(MysqlConnector* connector, int table_index, const char* field_name, const FieldType& value, char* format_buf, int format_buflen) {
		const MysqlTableFieldInfo* fi = get_field_info(table_index, field_name);
		if (!fi) return nullptr;
		if (!mysql_get_field_value_format(connector, fi->field_type, fi->create_flags, value, format_buf, format_buflen, nullptr, 0))
			return nullptr;
		return format_buf;
	}

private:
	typedef std::unordered_map<std::string, int> table_fields_name2index_type;

	std::string db_name_;
	table_fields_name2index_type table_name2index_;
	std::vector<const MysqlTableInfo*> table_array_;
	std::vector<table_fields_name2index_type> table_fields_name2index_array_;
};
