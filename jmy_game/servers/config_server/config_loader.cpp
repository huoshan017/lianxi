#include "config_loader.h"
#include <fstream>
#include <string>
#include <cassert>
#include <iostream>

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
	if (!doc_["server_name"].IsString()) {
		std::cout << "server_name type is not string" << std::endl;
		return false;
	}
	config_.name = doc_["server_name"].GetString();

	// server_id
	if (!doc_["server_id"].IsInt()) {
		std::cout << "server_id type is not int" << std::endl;
		return false;
	}
	config_.id = doc_["server_id"].GetInt();

	// server_ip
	if (!doc_["server_ip"].IsString()) {
		std::cout << "server_ip type is not string" << std::endl;
		return false;
	}
	config_.ip = doc_["server_ip"].GetString();

	// server_port
	if (!doc_["server_port"].IsInt()) {
		std::cout << "server_port type is not int" << std::endl;
		return false;
	}
	config_.port = (unsigned short)doc_["server_port"].GetInt();

	// max_conn
	if (!doc_["max_conn"].IsInt()) {
		std::cout << "max_conn type is not int" << std::endl;
		return false;
	}
	config_.max_conn = doc_["max_conn"].GetInt();

	// enable_reconnect
	if (!doc_["enable_reconnect"].IsBool()) {
		std::cout << "enable_reconnect type is not bool" << std::endl;
		return false;
	}
	config_.enable_reconnect = doc_["enable_reconnect"].GetBool();

	// gate_list_conf_path_type 
	if (!doc_["gate_list_conf_path_type"].IsInt()) {
		std::cout << "gate_list_conf_path_type type is not string" << std::endl;
		return false;
	}
	config_.gate_list_conf_path_type = doc_["gate_list_conf_path_type"].GetInt();

	// gate_list_conf_local_path 
	if (!doc_["gate_list_conf_local_path"].IsString()) {
		std::cout << "gate_list_conf_local_path type is not int" << std::endl;
		return false;
	}
	config_.gate_list_conf_local_path = doc_["gate_list_conf_local_path"].GetString();

	// gate_list_conf_remote_path 
	if (!doc_["gate_list_conf_remote_path"].IsString()) {
		std::cout << "gate_list_conf_remote_path type is not int" << std::endl;
		return false;
	}
	config_.gate_list_conf_remote_path = doc_["gate_list_conf_remote_path"].GetString();

	// log_conf_path 
	if (!doc_["log_conf_path"].IsString()) {
		std::cout << "log_conf_path type is not string" << std::endl;
		return false;
	}
	config_.log_conf_path = doc_["log_conf_path"].GetString();
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
