#pragma once

#include "../thirdparty/include/rapidjson/document.h"
#include "../thirdparty/include/rapidjson/stringbuffer.h"
#include "../thirdparty/include/rapidjson/writer.h"
#include "../mysql/mysql_defines.h"
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

	struct SelKeyInfo {
		std::string key;
		std::string result_type;
		SelKeyInfo(const std::string& k, const std::string& r) : key(k), result_type(r) {}
	};

	struct TableInfo {
		std::string name;
		std::string primary_key;
		int primary_key_index;
		std::string db_engine;
		std::vector<SelKeyInfo> select_keys_info;
		std::string update_key;
		std::string delete_key;
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
		std::string field_structs;
	};

private:
	bool parse_table_fields(const char* table_name, std::vector<FieldInfo>& fields);
	bool generate_define_file(std::fstream& out_file, const std::string& file_name);
	bool generate_struct_file(std::fstream& out_file, std::fstream& out_file2, const std::string& file_name);
	bool generate_func_file(std::fstream& out_file, std::fstream& out_file2, const std::string& file_name);
	//const char* get_field_format_for_func(const std::vector<FieldInfo>& field_vec, const std::string& field_name);

	bool gen_insert_record_func(std::fstream& out_file, std::fstream& out_file2, const TableInfo& table_info, std::vector<FieldInfo>& fields);
	bool gen_update_record_func(std::fstream& out_file, std::fstream& out_file2, const TableInfo& table_info, std::vector<FieldInfo>& fields, const std::string& update_key);
	bool gen_select_record_func(std::fstream& out_file, std::fstream& out_file2, int table_index, int select_key_index);
	bool gen_get_result_of_select_record_func(std::fstream& out_file, std::fstream& out_file2, int table_index, int select_key_index);
	bool gen_delete_record_func(std::fstream& out_file, std::fstream& out_file2, const TableInfo& table_info, std::vector<FieldInfo>& fields, const std::string& delete_key);

	//bool gen_db_tables_manager(std::fstream& out_file, std::fstream& out_file2);

private:
	rapidjson::Document doc_;
	Config config_;
	std::string jsonpath_;
};
