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
		ServerLogError("failed to open %s", jsonpath);
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
		ServerLogError("parse %s failed, err %d", jsonpath, doc_.GetParseError());
		return false;
	}

	// server_name
	if (!doc_["server_name"].IsString()) {
		ServerLogError("server_name type is not string");
		return false;
	}
	config_.name = doc_["server_name"].GetString();

	// server_id
	if (!doc_["server_id"].IsInt()) {
		ServerLogError("server_id type is not int");
		return false;
	}
	config_.id = doc_["server_id"].GetInt();

	// server_ip
	if (!doc_["server_ip"].IsString()) {
		ServerLogError("server_ip type is not string");
		return false;
	}
	config_.ip = doc_["server_ip"].GetString();

	// server_port
	if (!doc_["server_port"].IsInt()) {
		ServerLogError("server_port type is not int");
		return false;
	}
	config_.port = (short)doc_["server_port"].GetInt();

	// max_conn
	if (!doc_["max_conn"].IsInt()) {
		ServerLogError("max_conn type is not int");
		return false;
	}
	config_.max_conn = doc_["max_conn"].GetInt();

	// enable_reconnect
	if (!doc_["enable_reconnect"].IsBool()) {
		ServerLogError("enable_reconnect type is not bool");
		return false;
	}
	config_.enable_reconnect = doc_["enable_reconnect"].GetBool();

	// listen_gate_ip
	if (!doc_["listen_gate_ip"].IsString()) {
		ServerLogError("listen_gate_ip type is not string");
		return false;
	}
	config_.listen_gate_ip = doc_["listen_gate_ip"].GetString();

	// listen_gate_port
	if (!doc_["listen_gate_port"].IsInt()) {
		ServerLogError("listen_gate_port type is not int");
		return false;
	}
	config_.listen_gate_port = doc_["listen_gate_port"].GetInt();

	// listen_gate_max_conn
	if (!doc_["listen_gate_max_conn"].IsInt()) {
		ServerLogError("listen_gate_max_conn type is not int");
		return false;
	}
	config_.listen_gate_max_conn = doc_["listen_gate_max_conn"].GetInt();

	// listen_gate_enable_reconnect
	if (!doc_["listen_gate_enable_reconnect"].IsBool()) {
		ServerLogError("listen_gate_enable_reconnect type is not bool");
		return false;
	}
	config_.listen_gate_enable_reconnect = doc_["listen_gate_enable_reconnect"].GetBool();
	jsonpath_ = jsonpath;
	
	ServerLogInfo("load %s success", jsonpath);
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
