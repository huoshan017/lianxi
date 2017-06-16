#include "conf_gate_list.h"
#include "../libjmy/jmy_mem.h"
#include <fstream>
#include <iostream>

ConfGateList::ConfGateList() : gate_count_(0)
{
}

ConfGateList::~ConfGateList()
{
	clear();
}

bool ConfGateList::loadJson(const char* jsonpath)
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
	if (!doc_.IsArray()) {
		std::cout << "gate server list type is not array" << std::endl;
		return false;
	}

	std::string server_name;
	int server_id = 0;
	std::string server_ip;
	short server_port = 0;
	std::string platform;
	std::string channel_code;
	size_t s = doc_.Size();
	for (size_t i=0; i<s; ++i) {
		rapidjson::Value& a = doc_[i];
		if (!a["server_name"].IsString()) {
			std::cout << "server_name type is not string" << std::endl;
			return false;
		}
		server_name = a["server_name"].GetString();
		if (!a["server_id"].IsInt()) {
			std::cout << "server_id type is not int" << std::endl;
			return false;
		}
		server_id = a["server_id"].GetInt();
		if (!a["server_ip"].IsString()) {
			std::cout << "server_ip type is not string" << std::endl;
			return false;
		}
		server_ip = a["server_ip"].GetString();
		if (!a["server_port"].IsInt()) {
			std::cout << "server_port type is not int" << std::endl;
			return false;
		}
		server_port = (short)a["server_port"].GetInt();
		if (a.HasMember("platform")) {
			if (!a["platform"].IsString()) {
				std::cout << "server platform type is not string" << std::endl;
				return false;
			}
			platform = a["platform"].GetString();
		}
		if (a.HasMember("channel_code")) {
			if (!a["channel_code"].IsString()) {
				std::cout << "server channel_code type is not string" << std::endl;
				return false;
			}
			channel_code = a["channel_code"].GetString();
		}

		MsgGateConfData* data = gate_array_[gate_count_];
		if (!data) {
			data = jmy_mem_malloc<MsgGateConfData>();
			gate_array_[gate_count_++] = data;
		}
		data->set_id(server_id);
		data->set_name(server_name);
		data->set_ip(server_ip);
		data->set_port(server_port);
		data->set_platform(platform);
		data->set_channel_code(channel_code);
	}

	return true;
}

bool ConfGateList::reload()
{
	reset();
	return loadJson(jsonpath_.c_str());
}

void ConfGateList::clear()
{
	size_t i = 0;
	for (; i<gate_count_; ++i) {
		if (gate_array_[i]) {
			jmy_mem_free(gate_array_[i]);
			gate_array_[i] = nullptr;
		}
	}
	gate_count_ = 0;
}

void ConfGateList::reset()
{
	gate_count_ = 0;
}

MsgGateConfData* ConfGateList::get(int index)
{
	if (index >= (int)gate_count_)
		return nullptr;
	return gate_array_[index];
}

MsgGateConfData* ConfGateList::getById(int id)
{
	std::unordered_map<int, MsgGateConfData*>::iterator it =id2gate_map_.find(id);
	if (it == id2gate_map_.end())
		return nullptr;

	return it->second;
}
