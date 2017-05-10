#pragma once

#include "../thirdparty/include/rapidjson/document.h"
#include "../thirdparty/include/rapidjson/stringbuffer.h"
#include "../thirdparty/include/rapidjson/writer.h"
#include "../db_server/mysql_defines.h"
#include <string>
#include <vector>

class DBConfigParser
{
public:
	DBConfigParser();
	~DBConfigParser();

	bool load(const char* conf_file);
	void clear();
	bool generate();

	struct TableInfo {
		std::string name;
		std::string primary_key;
		int primary_key_index;
		std::string db_engine;
		std::vector<std::string> select_keys;
	};

	struct FieldInfo {
		std::string name;
		std::string field_type;
		int field_length;
		std::string index_type;
		std::string create_flags;
	};

	struct Config {
		std::string db_name;
		std::vector<std::string> struct_include_strings;
		std::vector<std::string> define_include_strings;
		std::vector<std::string> funcs_include_strings;
		std::vector<TableInfo> tables;
		std::vector<std::vector<FieldInfo> > tables_fields;
	};

private:
	bool parse_table_fields(const char* table_name, std::vector<FieldInfo>& fields);
	bool generate_defines_file(std::fstream& out_file, const std::string& file_name);
	bool generate_structs_file(std::fstream& out_file, const std::string& file_name);
	bool generate_funcs_file(std::fstream& out_file, std::fstream& out_file2, const std::string& file_name);
	const char* get_field_format_for_func(const std::vector<FieldInfo>& field_vec, const std::string& field_name);
	bool generate_insert_record_func(std::fstream& out_file, std::fstream& out_file2, const TableInfo& table_info, std::vector<FieldInfo>& fields);
	bool generate_update_record_func(std::fstream& out_file, std::fstream& out_file2, const TableInfo& table_info, std::vector<FieldInfo>& fields);

private:
	rapidjson::Document doc_;
	Config config_;
	std::string jsonpath_;
};
