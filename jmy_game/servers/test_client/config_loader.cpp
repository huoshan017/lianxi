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

	// server_ip
	const char* connect_ip = "connect_ip";
	if (doc_.HasMember(connect_ip)) {
		if (!doc_[connect_ip].IsString()) {
			std::cout << connect_ip << " type is not string" << std::endl;
			return false;
		}
		config_.connect_ip = doc_[connect_ip].GetString();
		std::cout << connect_ip << ": " << config_.connect_ip << std::endl;
	}

	// server_port
	const char* connect_port = "connect_port";
	if (doc_.HasMember(connect_port)) {
		if (!doc_[connect_port].IsInt()) {
			std::cout << connect_port << " type is not int" << std::endl;
			return false;
		}
		config_.connect_port = (unsigned short)doc_[connect_port].GetInt();
		std::cout << connect_port << ": " << config_.connect_port << std::endl;
	}

	// enable_reconnect
	const char* enable_reconnect = "enable_reconnect";
	if (doc_.HasMember(enable_reconnect)) {
		if (!doc_[enable_reconnect].IsBool()) {
			std::cout << enable_reconnect << " type is not bool" << std::endl;
			return false;
		}
		config_.enable_reconnect = doc_[enable_reconnect].GetBool();
		std::cout << enable_reconnect << ": " << config_.enable_reconnect << std::endl;
	}

	// log_conf
	const char* log_conf = "log_conf_path";
	if (doc_.HasMember(log_conf)) {
		if (!doc_[log_conf].IsString()) {
			std::cout << log_conf << " type is not string" << std::endl;
			return false;
		}
		config_.log_conf_path = doc_[log_conf].GetString();
		std::cout << log_conf << ": " << config_.log_conf_path << std::endl;
	}

	// account_prefix
	const char* account_prefix = "account_prefix";
	if (doc_.HasMember(account_prefix)) {
		if (!doc_[account_prefix].IsString()) {
			std::cout << account_prefix << " type is not string" << std::endl;
			return false;
		}
		config_.account_prefix = doc_[account_prefix].GetString();
		std::cout << account_prefix << ": " << config_.account_prefix << std::endl;
	}

	// account_start_index 
	const char* account_start_index = "account_start_index";
	if (doc_.HasMember(account_start_index)) {
		if (!doc_[account_start_index].IsInt()) {
			std::cout << account_start_index << " type is not int" << std::endl;
			return false;
		}
		config_.account_start_index = doc_[account_start_index].GetInt();
		std::cout << account_start_index << ": " << config_.account_start_index << std::endl;
	}

	// account_num
	const char* account_num = "account_num";
	if (doc_.HasMember(account_num)) {
		if (!doc_[account_num].IsInt()) {
			std::cout << account_num << " type is not int" << std::endl;
			return false;
		}
		config_.account_num = doc_[account_num].GetInt();
		std::cout << account_num << ": " << config_.account_num << std::endl;
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
