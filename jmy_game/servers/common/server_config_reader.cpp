#include "server_config_reader.h"
#include <fstream>
#include <string>
#include <cassert>
#include <iostream>

ServerConfigReader::ServerConfigReader()
{
}

ServerConfigReader::~ServerConfigReader()
{
	close();
}

bool ServerConfigReader::loadJson(const char* jsonpath)
{
	std::ifstream in;

	in.open(jsonpath, std::ifstream::in);
	if (!in.is_open()) {
		std::cerr << "failed to open " << jsonpath << std::endl;
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
		std::cerr << "parse " << jsonpath << " failed, err " << doc_.GetParseError() << std::endl;
		return false;
	}

	if (!parseServerNode()) {
		return false;
	}

	if (!parseClientNodes()) {
		return false;
	}
	
	std::cout << "load " << jsonpath << " success" << std::endl;
	return true;
}

void ServerConfigReader::close()
{
	doc_.Clear();
}

bool ServerConfigReader::parseServerNode()
{
	rapidjson::Value& v = doc_["server"];
	if (v.IsNull()) {
		std::cerr << "server node not found" << std::endl;
		return false;
	}

	if (!v["name"].IsString()) {
		std::cerr << "server name type is not string" << std::endl;
		return false;
	}
	server_node_.name = v["name"].GetString();

	if (!v["id"].IsInt()) {
		std::cerr << "server id type is not int" << std::endl;
		return false;
	}
	server_node_.id = v["id"].GetInt();

	if (!v["ip"].IsString()) {
		std::cerr << "server ip type is not string" << std::endl;
		return false;
	}
	server_node_.ip = v["ip"].GetString();

	if (!v["port"].IsInt()) {
		std::cerr << "server port type is not int" << std::endl;
		return false;
	}
	server_node_.port = (short)v["port"].GetInt();

	if (!v["max_conn"].IsInt()) {
		std::cerr << "server max_conn type is not int" << std::endl;
		return false;
	}
	server_node_.max_conn = v["max_conn"].GetInt();

	if (!v["enable_reconnect"].IsBool()) {
		std::cerr << "server enable_reconnect type is not bool" << std::endl;
		return false;
	}
	server_node_.support_reconnect = v["enable_reconnect"].GetBool();

	return true;
}

bool ServerConfigReader::parseClientNodes()
{
	rapidjson::Value& v = doc_["clients"];
	if (v.IsNull()) {
		return true;
	}

	if (!v.IsArray()) {
		std::cerr << "server clients type is not array" << std::endl;
		return false;
	}

	size_t s = sizeof(client_nodes_)/sizeof(client_nodes_[0]);
	for (size_t i=0; i<v.Size(); i++) {
		if (i >= s-1)
			break;
		rapidjson::Value& vv = v[i];
		if (!vv.IsObject()) {
			std::cerr << "clients child node type is not object" << std::endl;
			return false;
		}

		if (!vv["name"].IsString()) {
			std::cerr << "clients child node name type is not string" << std::endl;
			return false;
		} 
		client_nodes_[i].name = vv["name"].GetString();

		if (!vv["id"].IsInt()) {
			std::cerr << "clients child node id type is not int" << std::endl;
			return false;
		}
		client_nodes_[i].id = vv["id"].GetInt();

		if (!vv["ip"].IsString()) {
			std::cerr << "clients child node ip type is not string" << std::endl;
			return false;
		}
		client_nodes_[i].ip = vv["ip"].GetString();

		if (!vv["port"].IsInt()) {
			std::cerr << "clients child node port type is not int" << std::endl;
			return false;
		}
		client_nodes_[i].port = (int)vv["port"].GetInt();
	}

	return true;
}
