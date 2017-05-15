#include "db_config_parser.h"
#include <fstream>
#include <string>
#include <cassert>
#include <iostream>
#include <thread>
#include <boost/format.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#include "../mysql/mysql_defines.h"
#include "../mysql/mysql_util.h"
#include "db_field_config_parser.h"

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
	member = (char*)"define_include";
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
			std::cout << "define_include member type is not string" << std::endl;
			return false;
		}
		config_.define_include_strings.push_back(v.GetString());
	}

	// funcs_include
	member = (char*)"func_include";
	if (!doc_.HasMember(member)) {
		std::cout << member << "member not exist" << std::endl;
		return false;
	}
	if (!doc_[member].IsArray()) {
		std::cout << member << "member type is not array" << std::endl;
		return false;
	}
	s = doc_[member].Size();
	for (int i=0; i<s; ++i) {
		rapidjson::Value& v = doc_[member].GetArray()[i];
		if (!v.IsString()) {
			std::cout << "func_include member type is not string" << std::endl;
			return false;
		}
		config_.funcs_include_strings.push_back(v.GetString());
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

	// tables_operate_keys
	member = (char*)"tables_operate_keys";
	if (!doc_.HasMember(member)) {
		std::cout << member << " member not exist" << std::endl;
		return false;
	}
	if (!doc_[member].IsArray()) {
		std::cout << member << " type is not array" << std::endl;
		return false;
	}
	s = doc_[member].GetArray().Size();
	for (i=0; i<s; ++i) {
		rapidjson::Value& v = doc_[member].GetArray()[i];
		char* table_member = (char*)"select";
		if (!v.HasMember(table_member)) {
			std::cout << table_member << " member of table not exist" << std::endl;
			return false;
		}
		if (!v[table_member].IsString()) {
			std::cout << table_member << " member type is not string" << std::endl;
			return false;
		}
		std::string s = v[table_member].GetString();
		boost::split(config_.tables[i].select_keys, s, boost::is_any_of(","));

		table_member = (char*)"update";
		if (!v.HasMember(table_member)) {
			std::cout << table_member << " member of table not exist" << std::endl;
			return false;
		}
		if (!v[table_member].IsString()) {
			std::cout << table_member << " member type is not string" << std::endl;
			return false;
		}
		s = v[table_member].GetString();
		boost::split(config_.tables[i].update_keys, s, boost::is_any_of(","));

		table_member = (char*)"delete";
		if (!v.HasMember(table_member)) {
			std::cout << table_member << " member of table not exist" << std::endl;
			return false;
		}
		if (!v[table_member].IsString()) {
			std::cout << table_member << " member type is not string" << std::endl;
			return false;
		}
		s = v[table_member].GetString();
		boost::split(config_.tables[i].delete_keys, s, boost::is_any_of(","));
	}

	// field struct
	member = (char*)"field_structs";
	if (!doc_.HasMember(member)) {
		std::cout << member << " member not exist" << std::endl;
		return false;
	}
	std::string field_config_path = doc_[member].GetString();

	DBFieldConfigParser field_config_parser;
	if (!field_config_parser.load(field_config_path.c_str())) {
		std::cout << "load field config " << field_config_path << " failed" << std::endl;
		return false;
	}

	if (!field_config_parser.generate()) {
		std::cout << "generate field struct failed" << std::endl;
		return false;
	}

	field_config_parser.clear();
	
	jsonpath_ = jsonpath;
	std::cout << "load " << jsonpath << " success" << std::endl;
	return true;
}

void DBConfigParser::clear()
{
}

static const char* get_blob_user_define_data(const std::string& field_type) {
	const char* blob_str = "blob:";
	const char* binary_str = "binary:";

	static std::string ss;
	int idx = field_type.find(blob_str);
	if (idx >= 0) {
		ss = field_type.substr(idx+std::strlen(blob_str)).c_str();
		return ss.c_str();
	} else {
		idx = field_type.find(binary_str);
		if (idx >= 0) {
			ss = field_type.substr(idx+std::strlen(binary_str)).c_str();
			return ss.c_str();
		}
	}
	return nullptr;
}

static const char* get_field_type_str(const DBConfigParser::FieldInfo& field_info) {
	std::string s(field_info.field_type);
	const char* type_str = get_blob_user_define_data(s);
	if (type_str)
		return type_str;

	if (s == "tinyint" || s == "smallint" ||
		s == "mediumint" || s == "int") {
		if (field_info.create_flags.find("unsigned") != std::string::npos)
			return "unsigned int";
		else
			return "int";
	} else if (s == "bigint") {
		if (field_info.create_flags.find("unsigned") != std::string::npos)
			return "uint64_t";
		else
			return "int64_t";
	} else if (s == "float")
		return "float";
	else if (s == "double")
		return "double";
	else if (s == "varchar" || s == "char" || s == "tinytext" || s == "text"  || s == "mediumtext" || s == "longtext") {
		return "std::string";
	}
	return nullptr;
}

static const char* get_field_type_str(const std::vector<DBConfigParser::FieldInfo>& fields_info, const std::string& field_name) {
	std::vector<DBConfigParser::FieldInfo>::const_iterator it = fields_info.begin();
	for (; it!=fields_info.end(); ++it) {
		if (it->name == field_name)
			break;
	}
	if (it == fields_info.end())
		return nullptr;
	return get_field_type_str(*it);
}

static bool is_value_type_simple(const std::string& value_type) {
	if (value_type=="char" || value_type=="unsigned char" ||
		value_type=="int8_t" || value_type=="uint8_t" ||
		value_type=="short" || value_type=="unsigned short" ||
		value_type=="int16_t" || value_type=="uint16_t" ||
		value_type=="int32_t" || value_type=="uint32_t" ||
		value_type=="int" || value_type=="unsigned int" ||
		value_type=="int64_t" || value_type=="uint64_t" ||
		value_type=="long" || value_type=="unsigned long" ||
		value_type=="long long" || value_type=="unsigned long long" ||
		value_type=="float" || value_type=="double")
		return true;
	return false;
}

static const char* get_field_type_str_by_name(const std::vector<DBConfigParser::FieldInfo>& fields_info, const std::string& field_name) {
	std::vector<DBConfigParser::FieldInfo>::const_iterator it = fields_info.begin();
	for (; it!=fields_info.end(); ++it) {
		if (it->name == field_name)
			break;
	}
	if (it == fields_info.end()) {
		std::cout << "cant get field_info by name " << field_name << std::endl;
		return nullptr;
	}
	return get_field_type_str(*it);
}

static bool is_basic_field_type(const std::string& field_type) {
	if (field_type == "tinyint" || field_type == "smallint" || field_type == "mediumint" || field_type == "int" || field_type == "bigint" ||
		field_type == "float" || field_type == "double")
		return true;
	return false;
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
	else if (s.find("binary") != std::string::npos) return "MYSQL_FIELD_TYPE_BINARY";
	else if (s.find("varbinary") != std::string::npos) return "MYSQL_FIELD_TYPE_VARBINARY";
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
	return nullptr;
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
	} else if (s == "unique") {
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
			boost::trim(str_vec[i]);
			if (str_vec[i] != "") {
				std::cout << "str_vec[" << i << "]: " << str_vec[i] << " invalid" << std::endl;
				return nullptr;
			}
		}
	}
	return res.c_str();
}

static bool can_select_field_type(const std::string& field_type) {
	if (field_type == std::string("int") ||
		field_type == std::string("tinyint") ||
		field_type == std::string("smallint") ||
		field_type == std::string("mediumint") ||
		field_type == std::string("int") ||
		field_type == std::string("bigint") ||
		field_type.find("blob:") != std::string::npos ||
		field_type.find("text") != std::string::npos ||
		field_type == std::string("char") ||
		field_type == std::string("varchar") ||
		field_type == std::string("binary") ||
		field_type == std::string("varbinary")) {
		return true;
	}
	return false;
}

bool DBConfigParser::generate()
{
	// generate db struct file
	std::fstream out_file, out_file2;
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

	if (!generate_define_file(out_file, file_name))
		return false;

	if (!generate_struct_file(out_file, out_file2, file_name))
		return false;

	if (!generate_func_file(out_file, out_file2, file_name))
		return false;

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

static const char* get_field_type_default_value(const std::string& field_type) {
	if (field_type == "tinyint" || field_type == "smallint" || field_type == "mediumint" || field_type == "int" || field_type == "bigint")
		return "0";
	else if (field_type == "float" || field_type == "double")
		return "0.f";
	else
		return nullptr;
}

static void gen_get_field_func(std::fstream& out_file, const DBConfigParser::FieldInfo& field_info, const char* field_type_str) {
	out_file << "  " << field_type_str << " get_" << field_info.name << "() const {" << std::endl;
	out_file << "    return " << field_info.name << ";" << std::endl;
	out_file <<	"  }" << std::endl;
}

static void gen_get_mutable_field_func(std::fstream& out_file, const DBConfigParser::FieldInfo& field_info, const char* field_type_str) {
	out_file << "  " << field_type_str << "& get_mutable_" << field_info.name << "() {" << std::endl;
	out_file << "    return " << field_info.name << ";" << std::endl;
	out_file << "  }" << std::endl;
}

static void gen_get_const_obj_field_func(std::fstream& out_file, const DBConfigParser::FieldInfo& field_info, const char* field_type_str) {
	out_file << "  const " << field_type_str << "& get_" << field_info.name << "() const {" << std::endl;
	out_file << "    return " << field_info.name << ";" << std::endl;
	out_file << "  }" << std::endl;
}

static void gen_init_field_func(std::fstream& out_file, const DBConfigParser::FieldInfo& field_info, const char* field_type_str) {
	out_file << "  void init_" << field_info.name << "(" << field_type_str << " data) {" << std::endl;
	out_file << "    " << field_info.name << " = data;" << std::endl;
	out_file << "    " << field_info.name << "_state = MYSQL_TABLE_FIELD_STATE_INITED;" << std::endl;
	out_file << "  }" << std::endl;
}

static void gen_init_const_obj_field_func(std::fstream& out_file, const DBConfigParser::FieldInfo& field_info, const char* field_type_str) {
	out_file << "  void init_" << field_info.name << "(const " << field_type_str << "& data) {" << std::endl;
	out_file << "    " << field_info.name << " = data;" << std::endl;
	out_file << "    " << field_info.name << "_state = MYSQL_TABLE_FIELD_STATE_INITED;" << std::endl;
	out_file << "  }" << std::endl;
}

static void gen_set_field_func(std::fstream& out_file, const DBConfigParser::FieldInfo& field_info, const char* field_type_str) {
	out_file << "  void set_" << field_info.name << "(" << field_type_str << " data) {" << std::endl;
	out_file << "    " << field_info.name << " = data;" << std::endl;
	out_file << "    " << field_info.name << "_state = MYSQL_TABLE_FIELD_STATE_CHANGED;" << std::endl;
	out_file << "  }" << std::endl;
}

static void gen_set_const_obj_field_func(std::fstream& out_file, const DBConfigParser::FieldInfo& field_info, const char* field_type_str) {
	out_file << "  void set_" << field_info.name << "(const " << field_type_str << "& data) {" << std::endl;
	out_file << "    " << field_info.name << " = data;" << std::endl;
	out_file << "    " << field_info.name << "_state = MYSQL_TABLE_FIELD_STATE_CHANGED;" << std::endl;
	out_file << "  }" << std::endl;
}

static void gen_get_field_state_func(std::fstream& out_file, const DBConfigParser::FieldInfo& field_info) {
	out_file << "  MysqlTableFieldValueState get_" << field_info.name << "_state() const {" << std::endl;
	out_file << "    return " << field_info.name << "_state;" << std::endl;
	out_file << "  }" << std::endl;
}

static void gen_set_field_state_func(std::fstream& out_file, const DBConfigParser::FieldInfo& field_info) {
	out_file << "  void set_" << field_info.name << "_state(MysqlTableFieldValueState state) {" << std::endl;
	out_file << "    " << field_info.name << "_state = state;" << std::endl;
	out_file << "  }" << std::endl;
}

bool DBConfigParser::generate_struct_file(std::fstream& out_file, std::fstream& out_file2, const std::string& file_name)
{
	std::string db_struct_file_name = file_name + "_struct.h";
	out_file.open(db_struct_file_name, std::ios::out);
	std::string db_struct_cpp_file_name = file_name + "_struct.cpp";
	out_file2.open(db_struct_cpp_file_name, std::ios::out);

	//////// .h
	out_file << "// Generated by the db_generator.  DO NOT EDIT!" << std::endl;
	out_file << "// source: " << jsonpath_ << std::endl << std::endl;
	out_file << "#pragma once" << std::endl;
	out_file << "#include <string>" << std::endl;
	out_file << "#include <list>" << std::endl;
	out_file << "#include <vector>" << std::endl;
	out_file << "#include <unordered_map>" << std::endl;
	for (int i=0; i<(int)config_.struct_include_strings.size(); ++i) {
		out_file << "#include \"" << config_.struct_include_strings[i] << "\"" << std::endl;
	}
	out_file << std::endl;
	//////// .h

	//////// .cpp
	out_file2 << "// Generated by the db_generator.  DO NOT EDIT!" << std::endl;
	out_file2 << "// source: " << jsonpath_ << std::endl << std::endl;
	out_file2 << "#include \"" << db_struct_file_name << "\"" << std::endl;
	//////// .cpp

	for (int i=0; i<(int)config_.tables.size(); ++i) {
		std::vector<FieldInfo>& fi_vec = config_.tables_fields[i];
		std::vector<FieldInfo>::iterator it;
		//////// .h
		{
			out_file << std::endl << "class " << config_.tables[i].name << " {" << std::endl;
			out_file << "public:" << std::endl;
			// constructor declaration
			out_file << "  " << config_.tables[i].name << "();" << std::endl;
			// function declaration
			for (it=fi_vec.begin(); it!=fi_vec.end(); ++it) {
				FieldInfo& fi = *it;
				if (is_field_time_type(fi.field_type.c_str()))
					continue;
				const char* ft_str = get_field_type_str(fi);
				if (!ft_str) {
					std::cout << "get invalid field type " << fi.field_type.c_str() << std::endl;
					return false;
				}
				bool is_basic_type = is_basic_field_type(fi.field_type);
				// getter and setter
				if (is_basic_type) {
					gen_get_field_func(out_file, fi, ft_str);
					gen_init_field_func(out_file, fi, ft_str);
					gen_set_field_func(out_file, fi, ft_str);
				} else {
					gen_get_mutable_field_func(out_file, fi, ft_str);
					gen_get_const_obj_field_func(out_file, fi, ft_str);
					gen_init_const_obj_field_func(out_file, fi, ft_str);
					gen_set_const_obj_field_func(out_file, fi, ft_str);
				}
				gen_get_field_state_func(out_file, fi);
				gen_set_field_state_func(out_file, fi);
			}
			
			// update functions declaration
			out_file << std::endl;

			// members
			out_file << "private:" << std::endl;
			// field member
			for (it=fi_vec.begin(); it!=fi_vec.end(); ++it) {
				FieldInfo& fi = *it;
				if (!is_field_time_type(fi.field_type.c_str())) {
					const char* ft_str = get_field_type_str(fi);
					if (!ft_str) {
						std::cout << "get invalid field type " << fi.field_type.c_str() << std::endl;
						return false;
					}
					out_file << "  " << ft_str << " " << fi.name << ";" << std::endl;
				}
			}
			// field value is changed member
			it = fi_vec.begin();
			for (; it!=fi_vec.end(); ++it) {
				FieldInfo& fi = *it;
				if (is_field_time_type(fi.field_type.c_str()))
					continue;
				out_file << "  MysqlTableFieldValueState " << fi.name << "_state;" << std::endl;
			}
			out_file << "};" << std::endl << std::endl;
		}
		//////// .h

		//////// .cpp
		{
			// constructor: init member
			out_file2 << config_.tables[i].name << "::" << config_.tables[i].name << "() : " << std::endl;
			for (it=fi_vec.begin(); it!=fi_vec.end(); ++it) {
				const char* ds = get_field_type_default_value((*it).field_type);
				if (!ds) continue;
				out_file2 << "  " << (*it).name << "(" << ds << ")," << std::endl;
			}
			bool b = false;
			for (it=fi_vec.begin(); it!=fi_vec.end(); ++it) {
				FieldInfo& fi = *it;
				if (!is_field_time_type(fi.field_type.c_str())) {
					if (b) out_file2 << "," << std::endl;
					out_file2 << "  " << fi.name << "_state(MYSQL_TABLE_FIELD_STATE_IDLE)";
					b = true;
				}
			}
			out_file2 << "{" << std::endl << "}" << std::endl << std::endl;
			out_file2 << std::endl;
		}
		//////// .cpp
	}

	if (!gen_db_tables_manager(out_file, out_file2)) {
		return false;
	}

	out_file.flush();
	std::this_thread::sleep_for(std::chrono::seconds(1));
	out_file.close();
	std::cout << "generated " << db_struct_file_name << std::endl;

	out_file2.flush();
	std::this_thread::sleep_for(std::chrono::seconds(1));
	out_file2.close();
	std::cout << "generated " << db_struct_cpp_file_name << std::endl;
	return true;
}

bool DBConfigParser::generate_define_file(std::fstream& out_file, const std::string& file_name)
{
	// generate db define file
	std::string db_defines_file_name = file_name + "_define.h";
	out_file.open(db_defines_file_name, std::ios::out);
	out_file << "// Generated by the db_generator.  DO NOT EDIT!" << std::endl;
	out_file << "// source: " << jsonpath_ << std::endl;
	out_file << "#pragma once" << std::endl;
	int s = (int)config_.define_include_strings.size();
	for (int i=0; i<s; ++i) {
		out_file << "#include \"" << config_.define_include_strings[i] << "\"" << std::endl;
	}
	out_file << std::endl;

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

const char* DBConfigParser::get_field_format_for_func(const std::vector<FieldInfo>& field_vec, const std::string& field_name) {
	int i = 0;
	int s = field_vec.size();
	for (; i<s; ++i) {
		if (field_name == field_vec[i].name) {
			break;
		}
	}

	if (i >= s) {
		std::cout << "field name " << field_name << " not found in fields vector" << std::endl;
		return nullptr;
	}

	static char buf[32];
	if (field_vec[i].field_type == std::string("tinyint") ||
		field_vec[i].field_type == std::string("smallint") ||
		field_vec[i].field_type == std::string("mediumint") ||
		field_vec[i].field_type == std::string("int")) {
		if (field_vec[i].create_flags.find("unsigned") != std::string::npos) {
			std::snprintf(buf, sizeof(buf), "unsigned int %s", field_name.c_str());
		} else {
			std::snprintf(buf, sizeof(buf), "int %s", field_name.c_str());
		}
	} else if (field_vec[i].field_type == std::string("bigint")) {
		if (field_vec[i].create_flags.find("unsigned") != std::string::npos) {
			std::snprintf(buf, sizeof(buf), "uint64_t %s", field_name.c_str());
		} else {
			std::snprintf(buf, sizeof(buf), "int64_t %s", field_name.c_str());
		}
	} else if (field_vec[i].field_type.find("text") != std::string::npos ||
			field_vec[i].field_type == std::string("char") ||
			field_vec[i].field_type == std::string("varchar")) {
		std::snprintf(buf, sizeof(buf), "const std::string& %s", field_name.c_str());
	} else {
		std::cout << "unsupported field type " << field_vec[i].field_type << std::endl;
		return nullptr;
	}

	return buf;
}

bool DBConfigParser::gen_select_record_func(std::fstream& out_file, std::fstream& out_file2, int table_index, int select_key_index) {
	std::vector<FieldInfo>& fields = config_.tables_fields[table_index];
	std::string select_key = config_.tables[table_index].select_keys[select_key_index];
	std::vector<std::string> keys;
	boost::split(keys, select_key, boost::is_any_of("&"));

	int ks = keys.size();
	if (ks == 0) {
		std::cout << "select keys count is invalid(0)" << std::endl;
		return false;
	} else if (ks > MYSQL_MAX_KEYS_COUNT) {
		std::cout << "select keys count is great to max count(" << MYSQL_MAX_KEYS_COUNT << ")" << std::endl;
		return false;
	}

	// support count of select keys to 3
	char* ff[MYSQL_MAX_KEYS_COUNT];
	for (int i=0; i<ks&&i<MYSQL_MAX_KEYS_COUNT; ++i) {
		ff[i] = (char*)get_field_format_for_func(fields, keys[i]);
		if (!ff[i]) {
			std::cout << "get field " << keys[i] << " failed" << std::endl;
			return false;
		}
	}

	std::string func_title;
	if (ks == 1) {
		func_title = "bool db_select_" + config_.tables[table_index].name + "_fields_by_" + keys[0] +
			"(" + ff[0] + ", mysql_cmd_callback_func get_result_func, void* param, long param_l)";
	} else if (ks == 2) {
		func_title = "bool db_select_" + config_.tables[table_index].name + "_fields_by_" + keys[0] + "_and_" + keys[1] + 
			"(" + ff[1] + ", mysql_cmd_callback_func get_result_func, void* param, long param_l)";
	} else if (ks == 3) {
		func_title = "bool db_select_" + config_.tables[table_index].name + "_fields_by_" + keys[0] + "_and_" + keys[1] + "_and_" + 
			"(" + ff[2] + ", mysql_cmd_callback_func get_result_func, void* param, long param_l)";
	}
	out_file << func_title << ";" << std::endl;

	out_file2 << func_title << " {" << std::endl;
	out_file2 << "	static const char* s_sel_fields[] = { " << std::endl;
	for (int k=0; k<(int)config_.tables_fields[table_index].size(); ++k) {
		if (can_select_field_type(config_.tables_fields[table_index][k].field_type)) {
			out_file2 << "		\"" << config_.tables_fields[table_index][k].name << "\"," << std::endl;
		}
	}
	out_file2 << "	};" << std::endl;
	out_file2 << "	if (!DB_MGR.selectRecord(\"" << config_.tables[table_index].name << "\", \"";
	if (ks == 1) {
		out_file2 << keys[0] << "\", " << keys[0];
	} else if (ks == 2) {
		out_file2 << keys[0] << "\", " << keys[0] << ", \"" << keys[1] << "\", " << keys[1];
	} else if (ks == 3) {
		out_file2 << keys[0] << "\", " << keys[0] << ", \"" << keys[1] << "\", " << keys[1] << ", \"" << keys[2] << "\", " << keys[2];
	}
	out_file2 << ", s_sel_fields, sizeof(s_sel_fields)/sizeof(s_sel_fields[0]), get_result_func, param, param_l)) {" << std::endl;
	out_file2 << "		return false;" << std::endl;
	out_file2 << "	}" << std::endl;
	out_file2 << "	return true;" << std::endl;
	out_file2 << "}" << std::endl << std::endl;
	return true;
}

bool DBConfigParser::gen_get_result_of_select_record_func(std::fstream& out_file, std::fstream& out_file2, int table_index) {
	std::string func_title = "bool db_get_result_of_select_" + config_.tables[table_index].name + "(MysqlConnector::Result& res, " + config_.tables[table_index].name + "& data)";
	out_file << func_title << ";" << std::endl;

	out_file2 << func_title << " {" << std::endl;
	out_file2 << "	if (res.res_err != 0) {" << std::endl;
	out_file2 << "		LogError(\"result(%d) is not no error\", res.res_err);" << std::endl;
	out_file2 << "		return false;" << std::endl;
	out_file2 << "	}" << std::endl;
	out_file2 << "	if (res.num_rows()==0 || res.is_empty()) {" << std::endl;
	out_file2 << "		LogError(\"result is empty\");" << std::endl;
	out_file2 << "		return false;" << std::endl;
	out_file2 << "	}" << std::endl;
	out_file2 << "	char** datas = res.fetch();" << std::endl;
	int m = 0;
	for (int n=0; n<(int)config_.tables_fields[table_index].size(); ++n) {
		const std::string& field_type = config_.tables_fields[table_index][n].field_type;
		const std::string& create_flags = config_.tables_fields[table_index][n].create_flags;
		const std::string& field_name = config_.tables_fields[table_index][n].name;
		if (can_select_field_type(field_type)) {
			if (field_type == std::string("tinyint") || field_type == std::string("smallint") ||
					field_type == std::string("mediumint") || field_type == std::string("int")) {
				if (create_flags.find("unsigned") != std::string::npos) {
					out_file2 << "	data.init_" << field_name << "((unsigned int)std::strtoul(datas[" << m << "], nullptr, 10));" << std::endl;
				} else {
					out_file2 << "	data.init_" << field_name << "(std::atoi(datas[" << m << "]));" << std::endl;
				}
			} else if (field_type == std::string("bigint")) {
				if (create_flags.find("unsigned") != std::string::npos) {
					out_file2 << "	data.init_" << field_name << "((uint64_t)std::strtoull(datas[" << m << "], nullptr, 10));" << std::endl;
				} else {
					out_file2 << "	data.init_" << field_name << "((int64_t)std::strtoull(datas[" << m << "], nullptr, 10));" << std::endl;
				}
			} else if (field_type.find("blob")!=std::string::npos || field_type.find("binary")!=std::string::npos) {
				const char* user_define = get_blob_user_define_data(field_type);
				if (!user_define) {
					std::cout << "user define data from " << field_type << " not found" << std::endl;
					return false;
				}
				//out_file << "	" << user_define << " ud" << user_define << ";" << std::endl;
				out_file2 << "	if (!data.get_mutable_" << field_name
					<< "().ParseFromArray(datas[" << m << "], std::strlen(datas[" << m << "]))) {" << std::endl;
				out_file2 << "		LogError(\"user define " << user_define << " parse failed\");" << std::endl;
				out_file2 << "		return false;" << std::endl;
				out_file2 << "	}" << std::endl;
				//out_file << "	data." << field_name << " = ud" << user_define << ";" << std::endl;
			} else if (field_type.find("text") != std::string::npos || field_type == std::string("char") || field_type == std::string("varchar")) {
				out_file2 << "	data.init_" << field_name << "(datas[" << m << "]);" << std::endl;
			} else {
				std::cout << "unsupported field type " << field_type << std::endl;
				return false;
			}
			m += 1;
		}
	}
	out_file2 << "	return true;" << std::endl;
	out_file2 << "}" << std::endl << std::endl;
	return true;
}

bool DBConfigParser::generate_func_file(std::fstream& out_file, std::fstream& out_file2, const std::string& file_name)
{
	// generate db define file
	std::string db_funcs_file_name = file_name + "_func.h";
	out_file.open(db_funcs_file_name, std::ios::out);
	out_file << "// Generated by the db_generator.  DO NOT EDIT!" << std::endl;
	out_file << "// source: " << jsonpath_ << std::endl;
	out_file << "#pragma once" << std::endl;
	int s = (int)config_.funcs_include_strings.size();
	for (int i=0; i<s; ++i) {
		out_file << "#include \"" << config_.funcs_include_strings[i] << "\"" << std::endl;
	}
	out_file << std::endl;
	
	std::string db_funcs_file_cpp_name = file_name + "_func.cpp";
	out_file2.open(db_funcs_file_cpp_name, std::ios::out);
	out_file2 << "// Generated by the db_generator. DO NOT EDIT!" << std::endl;
	out_file2 << "// source: " << jsonpath_ << std::endl;
	out_file2 << "#include \"" << db_funcs_file_name << "\"" << std::endl; 
	out_file2 << "#include <cstdlib>" << std::endl;
	out_file2 << "#include <string>" << std::endl << std::endl;

	std::string func_title;
	s = config_.tables.size();
	for (int i=0; i<s; ++i) {
		int ss = config_.tables[i].select_keys.size();
		for (int j=0; j<ss; ++j) {
			if (!gen_select_record_func(out_file, out_file2, i, j)) {
				std::cout << "generate select record function failed, table_index(" << i << ")" << std::endl;
				return false;
			}
		}

		// get result of select funcs
		if (!gen_get_result_of_select_record_func(out_file, out_file2, i)) {
			return false;
		}

		if (!gen_insert_record_func(out_file, out_file2, config_.tables[i], config_.tables_fields[i])) {
			std::cout << "generate insert record function failed, table_index(" << i << ")" << std::endl;
			return false;
		}

		ss = config_.tables[i].update_keys.size();
		for (int j=0; j<ss; ++j) {
			if (!gen_update_record_func(out_file, out_file2, config_.tables[i], config_.tables_fields[i], config_.tables[i].update_keys[j])) {
				std::cout << "generate update record function failed, table_index(" << i << ")" << std::endl;
				return false;
			}
		}
		out_file << std::endl;
	}

	out_file.flush();
	std::this_thread::sleep_for(std::chrono::seconds(1));
	out_file.close();
	std::cout << "generated " << db_funcs_file_name << std::endl;

	out_file2.flush();
	std::this_thread::sleep_for(std::chrono::seconds(1));
	out_file2.close();
	std::cout << "generated " << db_funcs_file_cpp_name << std::endl;

	return true;
}

bool DBConfigParser::gen_insert_record_func(std::fstream& out_file, std::fstream& out_file2, const TableInfo& table_info, std::vector<FieldInfo>& fields)
{
	std::string func_title = "bool db_insert_" + std::string(table_info.name) + "_record(const " + table_info.name +
		"& data, mysql_cmd_callback_func get_last_insert_id_func, void* param, long param_l)";

	//////// .h
	out_file << func_title << ";" << std::endl;
	//////// .h

	//////// .cpp
	out_file2 << func_title << " {" << std::endl;
	std::string format_params_string;
	for (int i=0; i<(int)fields.size(); ++i) {
		if (i == table_info.primary_key_index)
			continue;

		const char* field_type_str = get_field_type_str(fields[i]);
		if (!field_type_str)
			continue;

		std::string nv = "nv_" + fields[i].name;
		out_file2 << "  MysqlFieldNameValue<" << field_type_str << "> " << nv
			<< "(\"" << fields[i].name << "\", data.get_" << fields[i].name << "()" << ");" << std::endl; 
		if (format_params_string == "") {
			format_params_string = nv;
		} else {
			format_params_string += (", " + nv);
		}
	}
	out_file2 << "  if (!DB_MGR.insertRecord(\"" << table_info.name << "\", get_last_insert_id_func, param, param_l, " << format_params_string << ")) {" << std::endl;
	out_file2 << "    std::cout << \"insert record failed\" << std::endl;" << std::endl;
	out_file2 << "    return false;" << std::endl;
	out_file2 << "  }" << std::endl;
	out_file2 << "  return true;" << std::endl;
	out_file2 << "}" << std::endl << std::endl;
	//////// .cpp
	
	return true;
}

bool DBConfigParser::gen_update_record_func(std::fstream& out_file, std::fstream& out_file2, const TableInfo& table_info, std::vector<FieldInfo>& fields, const std::string& update_key)
{
	std::vector<std::string> keys;
	boost::split(keys, update_key, boost::is_any_of("&"));
	int ks = keys.size();
	if (ks == 0 || ks >= MYSQL_MAX_KEYS_COUNT) {
		std::cout << "update keys count " << ks << " is invalid" << std::endl;
		return false;
	}

	std::string func_title;
	if (ks == 1) {
		func_title = "bool db_update_" + std::string(table_info.name) + "_record_by_" + keys[0] + "(" + table_info.name + "& data)";
	} else if (ks == 2) {
		func_title = "bool db_update_" + std::string(table_info.name) + "_record_by_" + keys[0] + "_and_" + keys[1] + "(" + table_info.name + "& data)";
	} else if (ks == 3) {
		func_title = "bool db_update_" + std::string(table_info.name) + "_record_by_" + keys[0] + "_and_" + keys[1] + "_and_" + keys[2] + "(" + table_info.name + "& data)";
	}

	//////// .h
	out_file << func_title << ";" << std::endl;
	//////// .h

	//////// .cpp
	std::string format_params_string;
	out_file2 << func_title << " {" << std::endl;
	for (int i=0; i<(int)fields.size(); ++i) {
		// primary key can not update
		if (i == table_info.primary_key_index)
			continue;
		
		const char* field_type_str = get_field_type_str(fields[i]);
		if (!field_type_str)
			continue;

		std::string field_name = "field_" + fields[i].name;
		out_file2 << "  char* " << field_name << " = nullptr;" << std::endl;
		out_file2 << "  if (data.get_" << fields[i].name << "_state() == MYSQL_TABLE_FIELD_STATE_CHANGED) {" << std::endl;
		out_file2 << "    " << field_name << " = (char*)\"" << fields[i].name << "\";" << std::endl;
		out_file2 << "  }" << std::endl;

		std::string nv = "nv_" + fields[i].name;
		out_file2 << "  MysqlFieldNameValue<" << field_type_str << "> " << nv
			<< "(\"" << fields[i].name << "\", data.get_" << fields[i].name << "()" << ");" << std::endl; 
		if (format_params_string == "") {
			format_params_string = nv;
		} else {
			format_params_string += (", " + nv);
		}
	}
	out_file2 << "  if (!DB_MGR.updateRecord(\"" << table_info.name << "\", ";
	if (ks > 0) {
		out_file2 << "\"" << keys[0] << "\", " << "data.get_" << keys[0] << "()";
	}
	if (ks > 1) {
		out_file2 << ", \"" << keys[1] << "\", " << "data.get_" << keys[1] << "()";
	}
	if (ks > 2) {
		out_file2 << ", \"" << keys[2] << "\", " << "data.get_" << keys[2] << "()";
	}
	out_file2 << ", " << format_params_string << ")) {" << std::endl;
	out_file2 << "    std::cout << \"update record failed\" << std::endl;" << std::endl;
	out_file2 << "    return false;" << std::endl;
	out_file2 << "  }" << std::endl;
	for (int i=0; i<(int)fields.size(); ++i) {
		// primary key can not update
		if (i == table_info.primary_key_index) continue;
		const char* field_type_str = get_field_type_str(fields[i]);
		if (!field_type_str) continue;
		out_file2 << "  if (data.get_" << fields[i].name << "_state() == MYSQL_TABLE_FIELD_STATE_CHANGED) {" << std::endl;
		out_file2 << "    data.set_" << fields[i].name << "_state(MYSQL_TABLE_FIELD_STATE_COMMITTING);" << std::endl;
		out_file2 << "  }" << std::endl;
	}
	out_file2 << "  return true;" << std::endl;
	out_file2 << "}" << std::endl << std::endl;
	//////// .cpp
	
	return true;
}

bool DBConfigParser::gen_delete_record_func(std::fstream& out_file, std::fstream& out_file2, const TableInfo& table_info, std::vector<FieldInfo>& fields, const std::string& delete_key)
{
	return true;
}

bool DBConfigParser::gen_db_tables_manager(std::fstream& out_file, std::fstream& out_file2)
{
	std::string class_name = config_.db_name + "_tables_manager";
	out_file << "// " << class_name << std::endl;
	out_file << "class " <<  class_name << " {" << std::endl;
	out_file << "public:" << std::endl;
	out_file << "  " << class_name << "();" << std::endl;
	out_file << "  ~" << class_name << "();" << std::endl << std::endl;

	out_file2 << "// " << class_name << std::endl;
	out_file2 << class_name << "::" << class_name << "()" << std::endl;
	out_file2 << "{" << std::endl;
	out_file2 << "}" << std::endl << std::endl;
	out_file2 << class_name << "::~" << class_name << "()" << std::endl;
	out_file2 << "{" << std::endl;
	out_file2 << "}" << std::endl << std::endl;

	for (int i=0; i<(int)config_.tables.size(); ++i) {
		std::vector<std::string>& select_keys = config_.tables[i].select_keys;
		for (int j=0; j<select_keys.size(); ++j) {
			std::string data = select_keys[j] + "_2_" + config_.tables[i].name;
			// get function
			std::string func_name = "get_" + config_.tables[i].name + "_by_" + select_keys[j] + "(" + get_field_format_for_func(config_.tables_fields[i], select_keys[j]) + ")";
			out_file << std::endl;
			out_file << "  " << config_.tables[i].name << "* " << func_name << ";" << std::endl;
			out_file2 << std::endl;
			out_file2 << config_.tables[i].name << "* " << class_name << "::" << func_name << " {" << std::endl;
			std::string field_type_str = get_field_type_str_by_name(config_.tables_fields[i], select_keys[j]);
			if (is_value_type_simple(field_type_str)) {
				out_file2 << "  auto it = " << data << ".find(" << select_keys[j] << ");" << std::endl;
			} else {
				out_file2 << "  auto it = " << data << ".find(const_cast<" << get_field_type_str(config_.tables_fields[i], select_keys[j]) << "&>(" << select_keys[j] << "));" << std::endl;
			}
			out_file2 << "  if (it == " << data << ".end()) {" << std::endl;
			out_file2 << "    return nullptr;" << std::endl;
			out_file2 << "  }" << std::endl;
			out_file2 << "  return it->second;" << std::endl;
			out_file2 << "}" << std::endl;

			// get new function
			func_name = "get_new_" + config_.tables[i].name + "_by_" + select_keys[j] + "(" + get_field_format_for_func(config_.tables_fields[i], select_keys[j]) + ")";
			out_file << "  " << config_.tables[i].name << "* " << func_name << ";" << std::endl;
			out_file2 << std::endl;
			out_file2 << config_.tables[i].name << "* " << class_name << "::" << func_name << " {" << std::endl;
			out_file2 << "  " << config_.tables[i].name << "* a = new " << config_.tables[i].name << ";" << std::endl;
			out_file2 << "  " << data << ".insert(std::make_pair(" << select_keys[j] << ", a));" << std::endl;
			out_file2 << "  return a;" << std::endl;
			out_file2 << "}" << std::endl;

			// delete function
			func_name = "delete_" + config_.tables[i].name + "_by_" + select_keys[j] + "(" + get_field_format_for_func(config_.tables_fields[i], select_keys[j]) + ")";
			out_file << "  bool " << func_name << ";" << std::endl;
			out_file2 << std::endl;
			out_file2 << "bool " << class_name << "::" << func_name << " {" << std::endl;
			if (is_value_type_simple(field_type_str)) {
				out_file2 << "  auto it = " << data << ".find(" << select_keys[j] << ");" << std::endl;
			} else {
				out_file2 << "  auto it = " << data << ".find(const_cast<" << get_field_type_str(config_.tables_fields[i], select_keys[j]) << "&>(" << select_keys[j] <<  "));" << std::endl;
			}
			out_file2 << "  if (it  == " << data << ".end()) return false;" << std::endl; 
			out_file2 << "  " << data << ".erase(it);" << std::endl;
			out_file2 << "  return true;" << std::endl;
			out_file2 << "}" << std::endl;

			// commit function
			func_name = "commit_" + config_.tables[i].name + "_value_changed(" + get_field_format_for_func(config_.tables_fields[i], select_keys[j]) + ")";
			out_file << "  void " << func_name << ";" << std::endl;
			out_file2 << std::endl;
			out_file2 << "void " << class_name << "::" << func_name << " {" << std::endl;
			out_file2 << "}" << std::endl;
			// update function
		}
	}
	out_file << std::endl;
	out_file << "private:" << std::endl;
	for (int i=0; i<(int)config_.tables.size(); ++i) {
		std::vector<std::string>& select_keys = config_.tables[i].select_keys;
		for (int j=0; j<select_keys.size(); ++j) {
			std::string data = select_keys[j] + "_2_" + config_.tables[i].name;
			std::string field_type_str = get_field_type_str_by_name(config_.tables_fields[i], select_keys[j]);
			out_file << "  std::unordered_map<" << field_type_str << ", " << config_.tables[i].name << "*> " << data << ";" << std::endl;
		}
		out_file << "  std::unordered_map<const char*, MysqlTableFieldValueState> " << config_.tables[i].name << "_changed_map;"
	}
	out_file << "};" << std::endl;
	return true;
}
