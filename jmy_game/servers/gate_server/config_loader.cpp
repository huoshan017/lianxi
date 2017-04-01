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
	config_.port = (short)doc_["server_port"].GetInt();

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

	// listen_game_ip 
	if (!doc_["listen_game_ip"].IsString()) {
		std::cout << "listen_game_ip type is not string" << std::endl;
		return false;
	}
	config_.listen_game_ip = doc_["listen_game_ip"].GetString();

	// listen_game_port 
	if (!doc_["listen_game_port"].IsInt()) {
		std::cout << "listen_game_port type is not int" << std::endl;
		return false;
	}
	config_.listen_game_port = doc_["listen_game_port"].GetInt();

	// listen_game_max_conn 
	if (!doc_["listen_game_max_conn"].IsInt()) {
		std::cout << "listen_game_max_conn type is not int" << std::endl;
		return false;
	}
	config_.listen_game_max_conn = doc_["listen_game_max_conn"].GetInt();

	// connect_login_ip
	if (!doc_["connect_login_ip"].IsString()) {
		std::cout << "connect_login_ip type is not bool" << std::endl;
		return false;
	}
	config_.connect_login_ip = doc_["connect_login_ip"].GetString();

	// connect_login_port
	if (!doc_["connect_login_port"].IsInt()) {
		std::cout << "connect_login_port type is not int" << std::endl;
		return false;
	}
	config_.connect_login_port = doc_["connect_login_port"].GetInt();

	// connect_login_enable_reconnect
	if (!doc_["connect_login_enable_reconnect"].IsBool()) {
		std::cout << "connect_login_enable_reconnect type is not bool" << std::endl;
		return false;
	}
	config_.connect_login_enable_reconnect = doc_["connect_login_enable_reconnect"].GetBool();
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
