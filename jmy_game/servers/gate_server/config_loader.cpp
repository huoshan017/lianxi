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
	clear();
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
	if (!doc_.HasMember("server_name")) {
		std::cout << "not found server_name item" << std::endl;
		return false;
	}
	if (!doc_["server_name"].IsString()) {
		std::cout << "server_name type is not string" << std::endl;
		return false;
	}
	config_.name = doc_["server_name"].GetString();

	// server_id
	if (!doc_.HasMember("server_id")) {
		std::cout << "not found server_id item" << std::endl;
		return false;
	}
	if (!doc_["server_id"].IsInt()) {
		std::cout << "server_id type is not int" << std::endl;
		return false;
	}
	config_.id = doc_["server_id"].GetInt();
	std::cout << "server_id is " << config_.id << std::endl;

	// server_ip
	if (!doc_.HasMember("server_ip")) {
		std::cout << "not found server_ip item" << std::endl;
		return false;
	}
	if (!doc_["server_ip"].IsString()) {
		std::cout << "server_ip type is not string" << std::endl;
		return false;
	}
	config_.ip = doc_["server_ip"].GetString();

	// server_port
	if (!doc_.HasMember("server_port")) {
		std::cout << "not found server_port item" << std::endl;
		return false;
	}
	if (!doc_["server_port"].IsInt()) {
		std::cout << "server_port type is not int" << std::endl;
		return false;
	}
	config_.port = (unsigned short)doc_["server_port"].GetInt();

	// max_conn
	if (!doc_.HasMember("max_conn")) {
		std::cout << "not found max_conn item" << std::endl;
		return false;
	}
	if (!doc_["max_conn"].IsInt()) {
		std::cout << "max_conn type is not int" << std::endl;
		return false;
	}
	config_.max_conn = doc_["max_conn"].GetInt();

	// enable_reconnect
	if (!doc_.HasMember("enable_reconnect")) {
		std::cout << "not found enable_reconnect item" << std::endl;
		return false;
	}
	if (!doc_["enable_reconnect"].IsBool()) {
		std::cout << "enable_reconnect type is not bool" << std::endl;
		return false;
	}
	config_.enable_reconnect = doc_["enable_reconnect"].GetBool();

	// listen_game_ip 
	if (!doc_.HasMember("listen_game_ip")) {
		std::cout << "not found listen_game_ip item" << std::endl;
		return false;
	}
	if (!doc_["listen_game_ip"].IsString()) {
		std::cout << "listen_game_ip type is not string" << std::endl;
		return false;
	}
	config_.listen_game_ip = doc_["listen_game_ip"].GetString();

	// listen_game_port 
	if (!doc_.HasMember("listen_game_port")) {
		std::cout << "not found listen_game_port item" << std::endl;
		return false;
	}
	if (!doc_["listen_game_port"].IsInt()) {
		std::cout << "listen_game_port type is not int" << std::endl;
		return false;
	}
	config_.listen_game_port = (unsigned short)doc_["listen_game_port"].GetInt();

	// listen_game_max_conn 
	if (!doc_.HasMember("listen_game_max_conn")) {
		std::cout << "not found listen_game_max_conn item" << std::endl;
		return false;
	}
	if (!doc_["listen_game_max_conn"].IsInt()) {
		std::cout << "listen_game_max_conn type is not int" << std::endl;
		return false;
	}
	config_.listen_game_max_conn = doc_["listen_game_max_conn"].GetInt();

	// connect_config_ip
	if (!doc_.HasMember("connect_config_ip")) {
		std::cout << "not found connect_config_ip item" << std::endl;
		return false;
	}
	if (!doc_["connect_config_ip"].IsString()) {
		std::cout << "connect_config_ip type is not bool" << std::endl;
		return false;
	}
	config_.connect_config_ip = doc_["connect_config_ip"].GetString();

	// connect_config_port
	if (!doc_.HasMember("connect_config_port")) {
		std::cout << "not found connect_config_port item" << std::endl;
		return false;
	}
	if (!doc_["connect_config_port"].IsInt()) {
		std::cout << "connect_config_port type is not int" << std::endl;
		return false;
	}
	config_.connect_config_port = (unsigned short)doc_["connect_config_port"].GetInt();

	// connect_config_enable_reconnect
	if (!doc_.HasMember("connect_config_enable_reconnect")) {
		std::cout << "not found connect_config_enable_reconnect item" << std::endl;
		return false;
	}
	if (!doc_["connect_config_enable_reconnect"].IsBool()) {
		std::cout << "connect_config_enable_reconnect type is not bool" << std::endl;
		return false;
	}
	config_.connect_config_enable_reconnect = doc_["connect_config_enable_reconnect"].GetBool();

	// log_conf_path
	if (doc_.HasMember("log_conf_path") && doc_["log_conf_path"].IsString()) {
		config_.log_conf_path = doc_["log_conf_path"].GetString();
	}

	jsonpath_ = jsonpath;
	
	std::cout << "load " << jsonpath << " success" << std::endl;
	return true;
}

bool ConfigLoader::reload()
{
	clear();
	return loadJson(jsonpath_.c_str());
}

void ConfigLoader::clear()
{
}
