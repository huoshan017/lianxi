#include "db_config_parser.h"
#include <fstream>
#include <string>
#include <cassert>
#include <iostream>
#include <thread>
#include "../common/util.h"

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
		return s.substr(idx+std::strlen(blob_str)).c_str();
	}

	if (s == "tinyint" || s == "smallint" ||
		s == "mediumint" || s == "int")
		return "int";
	else if (s == "bigint")
		return "int64_t";
	else if (s == "varchar" || s == "char" || s == "text")
		return "std::string";
	else
		return nullptr;
}

static bool is_field_time_type(const char* field_type) {
	std::string s(field_type);
	if (s == "timestamp" || s == "date" || s == "datetime") {
		return true;
	}
	return false;
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
		out_file << "#inlude \"" << config_.struct_include_strings[i] << "\"" << std::endl;
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
		out_file << "include \"" << config_.define_include_strings[i] << "\"" << std::endl;
	}

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
