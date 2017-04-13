#include "config_loader.h"
#include <fstream>
#include <string>
#include <cassert>
#include <iostream>
#include "../common/util.h"

ConfigLoader::ConfigLoader()
{
}

ConfigLoader::~ConfigLoader()
{
	close();
}

bool ConfigLoader::loadJson(const char* jsonpath)
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

	// server_name
	char* member = (char*)"name";
	if (!doc_.HasMember(member)) {
		std::cout << member << " member not exist" << std::endl;
		return false;
	}
	if (!doc_[member].IsString()) {
		std::cout << member << " type is not string" << std::endl;
		return false;
	}
	config_.name = doc_[member].GetString();

	// server_id
	member = (char*)"id";
	if (!doc_.HasMember(member)) {
		std::cout << member << " member not exist" << std::endl;
		return false;
	}
	if (!doc_[member].IsInt()) {
		std::cout << member << " type is not int" << std::endl;
		return false;
	}
	config_.id = doc_[member].GetInt();
	if (get_server_type(config_.id) != SERVER_TYPE_DB) {
		std::cout << member << " " << config_.id << " type is not config server type" << std::endl;
		return false;
	}

	// server_ip
	member = (char*)"ip";
	if (!doc_.HasMember(member)) {
		std::cout << member << " member not exist" << std::endl;
		return false;
	}
	if (!doc_[member].IsString()) {
		std::cout << member << " type is not string" << std::endl;
		return false;
	}
	config_.ip = doc_[member].GetString();

	// server_port
	member = (char*)"port";
	if (!doc_.HasMember(member)) {
		std::cout << member << " member not exist" << std::endl;
		return false;
	}
	if (!doc_[member].IsInt()) {
		std::cout << member << " type is not int" << std::endl;
		return false;
	}
	config_.port = (unsigned short)doc_[member].GetInt();

	// max_conn
	member = (char*)"max_conn";
	if (!doc_.HasMember(member)) {
		std::cout << member << " member not exist" << std::endl;
		return false;
	}
	if (!doc_[member].IsInt()) {
		std::cout << member << " type is not int" << std::endl;
		return false;
	}
	config_.max_conn = doc_[member].GetInt();

	// enable_reconnect
	member = (char*)"enable_reconnect";
	if (!doc_.HasMember(member)) {
		std::cout << member << " member not exist" << std::endl;
		return false;
	}
	if (!doc_[member].IsBool()) {
		std::cout << member << " type is not bool" << std::endl;
		return false;
	}
	config_.enable_reconnect = doc_[member].GetBool();

	// log_conf_path 
	member = (char*)"log_conf_path";
	if (!doc_.HasMember(member)) {
		std::cout << member << " member not exist" << std::endl;
		return false;
	}
	if (!doc_[member].IsString()) {
		std::cout << member << " type is not string" << std::endl;
		return false;
	}
	config_.log_conf_path = doc_[member].GetString();

	// mysql_host
	member = (char*)"mysql_host";
	if (!doc_.HasMember(member)) {
		std::cout << member << " member not exist" << std::endl;
		return false;
	}
	if (!doc_[member].IsString()) {
		std::cout << member << " type is not string" << std::endl;
		return false;
	}
	config_.mysql_host = doc_[member].GetString();

	// mysql_port 
	member = (char*)"mysql_port";
	if (!doc_.HasMember(member)) {
		std::cout << member << " member not exist" << std::endl;
		return false;
	}
	if (!doc_[member].IsInt()) {
		std::cout << member << " type is not int" << std::endl;
		return false;
	}
	config_.mysql_port = doc_[member].GetInt();

	// mysql_user 
	member = (char*)"mysql_user";
	if (!doc_.HasMember(member)) {
		std::cout << member << " member not exist" << std::endl;
		return false;
	}
	if (!doc_[member].IsString()) {
		std::cout << member << " type is not string" << std::endl;
		return false;
	}
	config_.mysql_user = doc_[member].GetString();

	// mysql_password
	member = (char*)"mysql_password";
	if (!doc_.HasMember(member)) {
		std::cout << member << " member not exist" << std::endl;
		return false;
	}
	if (!doc_[member].IsString()) {
		std::cout << member << " type is not string" << std::endl;
		return false;
	}
	config_.mysql_password = doc_[member].GetString();

	// mysql_dbname
	member = (char*)"mysql_dbname";
	if (!doc_.HasMember(member)) {
		std::cout << member << " member not exist" << std::endl;
		return false;
	}
	if (!doc_[member].IsString()) {
		std::cout << member << " type is not string" << std::endl;
		return false;
	}
	config_.mysql_dbname = doc_[member].GetString();
	
	jsonpath_ = jsonpath;
	std::cout << "load " << jsonpath << " success" << std::endl;
	return true;
}

bool ConfigLoader::reload()
{
	//doc_.Clear();
	return loadJson(jsonpath_.c_str());
}

void ConfigLoader::close()
{
	//doc_.Clear();
}
