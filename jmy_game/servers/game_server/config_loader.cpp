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
	if (doc_.HasMember("name")) {
		if (!doc_["name"].IsString()) {
			std::cout << "name type is not string" << std::endl;
			return false;
		}
		config_.name = doc_["name"].GetString();
	}

	// server_id
	if (doc_.HasMember("id")) {
		if (!doc_["id"].IsInt()) {
			std::cout << "id type is not int" << std::endl;
			return false;
		}
		config_.id = doc_["id"].GetInt();
		if (get_server_type(config_.id) != SERVER_TYPE_GAME) {
			std::cout << "server_id " << config_.id << " type is not config server type" << std::endl;
			return false;
		}
	}	

	// server_ip
	if (doc_.HasMember("ip")) {
		if (!doc_["ip"].IsString()) {
			std::cout << "ip type is not string" << std::endl;
			return false;
		}
		config_.ip = doc_["ip"].GetString();
	}

	// server_port
	if (doc_.HasMember("port")) {
		if (!doc_["port"].IsInt()) {
			std::cout << "port type is not int" << std::endl;
			return false;
		}
		config_.port = (unsigned short)doc_["port"].GetInt();
	}

	// max_conn
	if (doc_.HasMember("max_conn")) {
		if (!doc_["max_conn"].IsInt()) {
			std::cout << "max_conn type is not int" << std::endl;
			return false;
		}
		config_.max_conn = doc_["max_conn"].GetInt();
	}

	// enable_reconnect
	if (doc_.HasMember("enable_reconnect")) {
		if (!doc_["enable_reconnect"].IsBool()) {
			std::cout << "enable_reconnect type is not bool" << std::endl;
			return false;
		}
		config_.enable_reconnect = doc_["enable_reconnect"].GetBool();
	}

	// connect_gate_ip
	if (doc_.HasMember("connect_gate_ip")) {
		if (!doc_["connect_gate_ip"].IsInt()) {
			std::cout << "connect_gate_ip type is not string" << std::endl;
			return false;
		}
		config_.connect_gate_ip = doc_["connect_gate_ip"].GetInt();
	}

	// connect_gate_port 
	if (doc_.HasMember("connect_gate_port")) {
		if (!doc_["connect_gate_port"].IsString()) {
			std::cout << "connect_gate_port type is not int" << std::endl;
			return false;
		}
		config_.connect_gate_port = (unsigned short)doc_["connect_gate_port"].GetInt();
	}

	// connect_gate_enable_reconnect 
	if (doc_.HasMember("connect_gate_enable_reconnect")) {
		if (!doc_["connect_gate_enable_reconnect"].IsBool()) {
			std::cout << "connect_gate_enable_reconnect type is not bool" << std::endl;
			return false;
		}
		config_.connect_gate_enable_reconnect = doc_["connect_gate_enable_reconnect"].GetString();
	}

	// log_conf_path 
	if (doc_.HasMember("log_conf_path")) {
		if (!doc_["log_conf_path"].IsString()) {
			std::cout << "log_conf_path type is not string" << std::endl;
			return false;
		}
		config_.log_conf_path = doc_["log_conf_path"].GetString();
	}
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
