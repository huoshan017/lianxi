#pragma once

#include <string>
#include <unordered_map>
#include <vector>

struct MysqlDatabaseConfig;
struct MysqlTableInfo;
struct MysqlTableFieldInfo;
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
	const char* get_field_type_format(int table_index, const char* field_name);

private:
	typedef std::unordered_map<std::string, int> table_fields_name2index_type;

	std::string db_name_;
	std::unordered_map<std::string, int> table_name2index_;
	std::vector<const MysqlTableInfo*> table_array_;
	std::vector<table_fields_name2index_type> table_fields_name2index_array_;
};
