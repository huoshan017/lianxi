#pragma once

#include "../thirdparty/include/rapidjson/document.h"
#include "../thirdparty/include/rapidjson/stringbuffer.h"
#include "../thirdparty/include/rapidjson/writer.h"
#include "../libjmy/jmy_singleton.hpp"
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
		std::vector<TableInfo> tables;
		std::vector<std::vector<FieldInfo> > tables_fields;
	};

private:
	bool parse_table_fields(const char* table_name, std::vector<FieldInfo>& fields);

private:
	rapidjson::Document doc_;
	Config config_;
	std::string jsonpath_;
};
