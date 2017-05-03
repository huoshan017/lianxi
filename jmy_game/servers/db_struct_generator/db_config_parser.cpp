#include "db_config_parser.h"
#include <fstream>
#include <string>
#include <cassert>
#include <iostream>
#include <thread>
#include <boost/format.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#include "../common/util.h"
#include "../db_server/mysql_defines.h"

DBConfigParser::DBConfigParser()
{
}

DBConfigParser::~DBConfigParser()
{
}

bool DBConfigParser::load(const char* jsonpath)
{
	std::ifstream in;

	in.open(jsonpath, std::ifstream::in);
	if (!in.is_open()) {
		std::cout << "failed to open " << jsonpath << std::endl;
		return false;
	}

	std::string line;
	std::string str;
	while (std::getline(in, line)) {
		str.append(line+"\n");
	}
	in.close();

	doc_.Parse<0>(str.c_str());
	if (doc_.HasParseError()) {
		std::cout << "parse " << jsonpath << " failed, err " << doc_.GetParseError() << std::endl;
		return false;
	}

	// db_name
	char* member = (char*)"db_name";
	if (!doc_.HasMember(member)) {
		std::cout << member << " member not exist" << std::endl;
		return false;
	}
	if (!doc_[member].IsString()) {
		std::cout << member << " type is not string" << std::endl;
		return false;
	}
	config_.db_name = doc_[member].GetString();

	// struct_include
	member = (char*)"struct_include";
	if (!doc_.HasMember(member)) {
		std::cout << member << " member not exist" << std::endl;
		return false;
	}
	if (!doc_[member].IsArray()) {
		std::cout << member << " member type is not array" << std::endl;
		return false;
	}
	int s = doc_[member].Size();
	for (int i=0; i<s; ++i) {
		rapidjson::Value& v = doc_[member].GetArray()[i];
		if (!v.IsString()) {
			std::cout << "struct_include member type is not string" << std::endl;
			return false;
		}
		config_.struct_include_strings.push_back(v.GetString());
	}

	// defines_include
	member = (char*)"defines_include";
	if (!doc_.HasMember(member)) {
		std::cout << member << " member not exist" << std::endl;
		return false;
	}
	if (!doc_[member].IsArray()) {
		std::cout << member << " member type is not array" << std::endl;
		return false;
	}
	s = doc_[member].Size();
	for (int i=0; i<s; ++i) {
		rapidjson::Value& v = doc_[member].GetArray()[i];
		if (!v.IsString()) {
			std::cout << "defines_include member type is not string" << std::endl;
			return false;
		}
		config_.define_include_strings.push_back(v.GetString());
	}

	// tables
	member = (char*)"tables";
	if (!doc_.HasMember(member)) {
		std::cout << member << " member not exist" << std::endl;
		return false;
	}
	if (!doc_[member].IsArray()) {
		std::cout << member << " type is not array" << std::endl;
		return false;
	}
	s = doc_[member].GetArray().Size();
	config_.tables.resize(s);
	config_.tables_fields.resize(s);
	int i = 0;
	for (; i<s; ++i) {
		rapidjson::Value& v = doc_[member].GetArray()[i];
		char* table_member = (char*)"name";
		if (!v.HasMember(table_member)) {
			std::cout << table_member << " member of table not exist" << std::endl;
			return false;
		}
		config_.tables[i].name = v[table_member].GetString();

		// parse table
		if (!parse_table_fields(config_.tables[i].name.c_str(), config_.tables_fields[i])) {
			std::cout << "parse table " << table_member << " failed" << std::endl;
			return false;
		}

		table_member = (char*)"primary_key";
		if (!v.HasMember(table_member)) {
			std::cout << table_member << " member of table not exist" << std::endl;
			return false;
		}
		config_.tables[i].primary_key = v[table_member].GetString();

		table_member = (char*)"primary_key_index";
		if (!v.HasMember(table_member)) {
			std::cout << table_member << " member of table not exist" << std::endl;
			return false;
		}
		config_.tables[i].primary_key_index = v[table_member].GetInt();

		table_member = (char*)"engine";
		if (!v.HasMember(table_member)) {
			std::cout << table_member << " member of table not exist" << std::endl;
			return false;
		}
		config_.tables[i].db_engine = v[table_member].GetString();
	}
	
	jsonpath_ = jsonpath;
	std::cout << "load " << jsonpath << " success" << std::endl;
	return true;
}

void DBConfigParser::clear()
{
}

static const char* get_field_type_str(const char* field_type) {
	std::string s(field_type);
	const char* blob_str = "blob:";
	int idx = s.find(blob_str);
	if (idx >= 0) {
		static std::string ss;
		ss = s.substr(idx+std::strlen(blob_str)).c_str();
		return ss.c_str();
	}

	if (s == "tinyint" || s == "smallint" ||
		s == "mediumint" || s == "int")
		return "int";
	else if (s == "bigint")
		return "int64_t";
	else if (s == "float")
		return "float";
	else if (s == "double")
		return "double";
	else if (s == "varchar" || s == "char" || s == "tinytext" || s == "text" || s == "longtext")
		return "std::string";
	else
		return nullptr;
}

static bool is_field_time_type(const char* field_type) {
	std::string s(field_type);
	if (s == "timestamp" || s == "time" || s == "year" || s == "date" || s == "datetime") {
		return true;
	}
	return false;
}

static const char* get_field_type_define_str(const char* field_type) {
	std::string s(field_type);
	if (s == "tinyint") return "MYSQL_FIELD_TYPE_TINYINT";
	else if (s == "smallint") return "MYSQL_FIELD_TYPE_SMALLINT";
	else if (s == "mediumint") return "MYSQL_FIELD_TYPE_MEDIUMINT";
	else if (s == "int") return "MYSQL_FIELD_TYPE_INT";
	else if (s == "bigint") return "MYSQL_FIELD_TYPE_BIGINT";
	else if (s == "float") return "MYSQL_FIELD_TYPE_FLOAT";
	else if (s == "double") return "MYSQL_FIELD_TYPE_DOUBLE";
	else if (s == "char") return "MYSQL_FIELD_TYPE_CHAR";
	else if (s == "varchar") return "MYSQL_FIELD_TYPE_VARCHAR";
	else if (s == "text") return "MYSQL_FIELD_TYPE_TEXT";
	else if (s == "tinytext") return "MYSQL_FIELD_TYPE_TINYTEXT";
	else if (s == "longtext") return "MYSQL_FIELD_TYPE_LONGTEXT";
	else if (s == "mediumtext") return "MYSQL_FIELD_TYPE_MEDIUMTEXT";
	else if (s.find("tinyblob") != std::string::npos) return "MYSQL_FIELD_TYPE_TINYBLOB";
	else if (s.find("mediumblob") != std::string::npos) return "MYSQL_FIELD_TYPE_MEDIUM_BLOB";
	else if (s.find("blob") != std::string::npos) return "MYSQL_FIELD_TYPE_BLOB";
	else if (s.find("longblob") != std::string::npos) return "MYSQL_FIELD_TYPE_LONGBLOB";
	else if (s == "time") return "MYSQL_FIELD_TYPE_TIME";
	else if (s == "timestamp") return "MYSQL_FIELD_TYPE_TIMESTAMP";
	else if (s == "date") return "MYSQL_FIELD_TYPE_DATE";
	else if (s == "datetime") return "MYSQL_FIELD_TYPE_DATETIME";
	else if (s == "year") return "MYSQL_FIELD_TYPE_YEAR";
	else if (s == "enum") return "MYSQL_FIELD_TYPE_ENUM";
	else if (s == "set") return "MYSQL_FIELD_TYPE_SET";
	else return nullptr;
}

static const char* get_field_length_define_str(int field_length) {
	static char buf[16];
	if (field_length == 0)
		return "MYSQL_FIELD_DEFAULT_LENGTH";
	else {
		std::snprintf(buf, sizeof(buf), "%d", field_length);
		return buf;
	}
}

static const char* get_field_index_type_define_str(const char* field_index_type) {
	std::string s(field_index_type);
	if (s == "none") {
		return "MYSQL_INDEX_TYPE_NONE"; 
	} else if (s == "index") {
		return "MYSQL_INDEX_TYPE_NORMAL";
	} else if (s == "unique index") {
		return "MYSQL_INDEX_TYPE_UNIQUE";
	} else {
		return nullptr;
	}
}

static const char* get_create_flags_define_str(const char* create_flags) {
	std::string s(create_flags);
	std::vector<std::string> str_vec;
	boost::split(str_vec, s, boost::is_any_of(","));

	static std::string res;
	res = "";
	for (size_t i=0; i<str_vec.size(); ++i) {
		if (i > 0) { res += "|"; }
		if (str_vec[i] == "auto_increment") {
			res += "MYSQL_TABLE_CREATE_AUTOINCREMENT";
		} else if (str_vec[i] == "unsigned") {
			res += "MYSQL_TABLE_CREATE_UNSIGNED";
		} else if (str_vec[i] == "null") {
			res += "MYSQL_TABLE_CREATE_NULL";
		} else if (str_vec[i] == "not null") {
			res += "MYSQL_TABLE_CREATE_NOT_NULL";
		} else if (str_vec[i] == "current_timestamp") {
			res += "MYSQL_TABLE_CREATE_CURRENTTIMESTAMP";
		} else if (str_vec[i] == "current_timestamp_on_update") {
			res += "MYSQL_TABLE_CREATE_CURRENTTIMESTAMP_ON_UPDATE";
		} else if (str_vec[i] == "zerofill") {
			res += "MYSQL_TABLE_CREATE_ZEROFILL";
		} else if (str_vec[i] == "default") {
			res += "MYSQL_TABLE_CREATE_DEFAULT";
		} else {
			return nullptr;
		}
	}
	return res.c_str();
}

bool DBConfigParser::generate()
{
	// generate db struct file
	std::fstream out_file;
	int index = jsonpath_.find_last_of("/");
	if (index < 0) {
		std::cout << "db config json path " << jsonpath_.c_str() << " not valid" << std::endl;
		return false;
	}
	std::string file_name = jsonpath_.substr(index+1);
	index = file_name.find(".");
	if (index > 0) {
		file_name = file_name.substr(0, index);
	}
	std::cout << "file name is " << file_name.c_str() << std::endl;

	std::string db_struct_file_name = file_name + "_struct.h";
	out_file.open(db_struct_file_name, std::ios::out);
	out_file << "#pragma once" << std::endl;
	out_file << "#include <string>" << std::endl;
	for (int i=0; i<(int)config_.struct_include_strings.size(); ++i) {
		out_file << "#include \"" << config_.struct_include_strings[i] << "\"" << std::endl;
	}
	for (int i=0; i<(int)config_.tables.size(); ++i) {
		out_file << std::endl << "struct " << config_.tables[i].name << " {" << std::endl;
		std::vector<FieldInfo>& fi_vec = config_.tables_fields[i];
		std::vector<FieldInfo>::iterator it = fi_vec.begin();
		for (; it!=fi_vec.end(); ++it) {
			FieldInfo& fi = *it;
			if (!is_field_time_type(fi.field_type.c_str())) {
				const char* ft_str = get_field_type_str(fi.field_type.c_str());
				if (!ft_str) {
					std::cout << "get invalid field type " << fi.field_type.c_str() << std::endl;
					return false;
				}
				out_file << "  " << ft_str << " " << fi.name << ";" << std::endl;
			}
		}
		out_file << "};" << std::endl; 
	}
	out_file.flush();
	std::this_thread::sleep_for(std::chrono::seconds(1));
	out_file.close();
	std::cout << "generated " << db_struct_file_name << std::endl;

	// generate db define file
	std::string db_defines_file_name = file_name + "_defines.h";
	out_file.open(db_defines_file_name, std::ios::out);
	out_file << "#pragma once" << std::endl;
	int s = (int)config_.define_include_strings.size();
	for (int i=0; i<s; ++i) {
		out_file << "#include \"" << config_.define_include_strings[i] << "\"" << std::endl;
	}

	std::vector<std::string> table_name_list;
	s = config_.tables.size();
	for (int i=0; i<s; ++i) {
		std::string table_field_variable_name = config_.tables[i].name + "_table_fields_info";
		out_file << "static const MysqlTableFieldInfo " << table_field_variable_name << "[] = {" << std::endl;
		std::vector<FieldInfo>& f = config_.tables_fields[i];
		for (int j=0; j<(int)f.size(); ++j) {
			const char* type_ds = get_field_type_define_str(f[j].field_type.c_str());
			if (!type_ds) {
				std::cout << "get_field_type_define_str(" << f[j].field_type << ") failed" << std::endl;
				return false;
			}
			const char* length_ds = get_field_length_define_str(f[j].field_length);
			if (!length_ds) {
				std::cout << "get_field_length_define_str(" << f[j].field_length << ") failed" << std::endl;
				return false;
			}
			const char* index_ds = get_field_index_type_define_str(f[j].index_type.c_str());
			if (!index_ds) {
				std::cout << "get_field_index_type_define_str(" << f[j].index_type << ") failed" << std::endl;
				return false;
			}
			const char* create_flags_ds = get_create_flags_define_str(f[j].create_flags.c_str());
			if (!create_flags_ds) {
				std::cout << "get_create_flags_define_str(" << f[j].create_flags << ") failed" << std::endl;
				return false;
			}
			out_file << "  { \"" << f[j].name << "\", " << type_ds << ", " << length_ds << ", " << index_ds << ", " << create_flags_ds << " }," << std::endl;
		}
		out_file << "};" << std::endl;

		std::string table_variable_name = config_.tables[i].name + "_table_info";
		out_file << "static const MysqlTableInfo " << table_variable_name << " = {" << std::endl;
		out_file << "  \"" << config_.tables[i].name << "\"," << std::endl;
		out_file << "  " << config_.tables[i].primary_key_index << "," << std::endl;
		out_file << "  \"" << config_.tables[i].primary_key << "\"," << std::endl;
		out_file << "  " << table_field_variable_name << "," << std::endl;
		out_file << "  sizeof(" << table_field_variable_name << ")/sizeof(" << table_field_variable_name << "[0])," << std::endl;

		if (config_.tables[i].db_engine == "myisam") {
			out_file << "  " << "MYSQL_ENGINE_MYISAM," << std::endl;
		} else if (config_.tables[i].db_engine == "innodb") {
			out_file << "  " << "MYSQL_ENGINE_INNODB," << std::endl;
		} else {
			std::cout << "invalid db_engine " << config_.tables[i].db_engine << std::endl;
			return false;
		}
		out_file << "};" << std::endl;
		table_name_list.push_back(table_variable_name);
	}

	std::string table_infos_str = config_.db_name + "_tables";
	out_file << "static const MysqlTableInfo " << table_infos_str << "[] = {" << std::endl;
	for (int i=0; i<(int)table_name_list.size(); ++i) {
		out_file << "  " << table_name_list[i] << "," << std::endl;
	}
	out_file << "};" << std::endl;

	out_file << "static const MysqlDatabaseConfig s_" << config_.db_name << "_db_config = {" << std::endl;
	out_file << "  \"" << config_.db_name << "\"," << std::endl;
	out_file << "  " << table_infos_str << "," << std::endl;
	out_file << "  sizeof(" << table_infos_str << ")/sizeof(" << table_infos_str << "[0])," << std::endl;
	out_file << "};" << std::endl;

	out_file.flush();
	std::this_thread::sleep_for(std::chrono::seconds(1));
	out_file.close();
	std::cout << "generated " << db_defines_file_name << std::endl;

	return true;
}

bool DBConfigParser::parse_table_fields(const char* table_name, std::vector<DBConfigParser::FieldInfo>& fields)
{
	if (!doc_.HasMember(table_name)) {
		std::cout << "member table " << table_name << " not found" << std::endl;
		return false;
	}

	if (!doc_[table_name].IsArray()) {
		std::cout << "member table " << table_name << " not array type" << std::endl;
		return false;
	}

	int s = doc_[table_name].GetArray().Size();
	fields.resize(s);
	for (int i=0; i<s; i++) {
		rapidjson::Value& v = doc_[table_name].GetArray()[i];
		char* table_member = (char*)"name";
		if (!v.HasMember(table_member)) {
			std::cout << "table " << table_name << " not found member " << table_member << std::endl;
			return false;
		}
		fields[i].name = v[table_member].GetString();

		table_member = (char*)"field_type";
		if (!v.HasMember(table_member)) {
			std::cout << "table " << table_name << " not found member " << table_member << std::endl;
			return false;
		}
		fields[i].field_type = v[table_member].GetString();

		table_member = (char*)"field_length";
		if (!v.HasMember(table_member)) {
			std::cout << "table " << table_name << " not found member " << table_member << std::endl;
			return false;
		}
		fields[i].field_length = v[table_member].GetInt();

		table_member = (char*)"index_type";
		if (!v.HasMember(table_member)) {
			std::cout << "table " << table_name << " not found member " << table_member << std::endl;
			return false;
		}
		fields[i].index_type = v[table_member].GetString();

		table_member = (char*)"create_flags";
		if (!v.HasMember(table_member)) {
			std::cout << "table " << table_name << " not found member " << table_member << std::endl;
			return false;
		}
		fields[i].create_flags = v[table_member].GetString();
	} 
	return true;
}
