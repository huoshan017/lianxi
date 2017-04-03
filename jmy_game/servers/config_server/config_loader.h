#pragma once

#include "../thirdparty/include/rapidjson/document.h"
#include "../thirdparty/include/rapidjson/stringbuffer.h"
#include "../thirdparty/include/rapidjson/writer.h"

class ConfigLoader
{
public:
	ConfigLoader();
	~ConfigLoader();

	bool loadJson(const char* jsonpath);
	bool reload();
	void close();

	struct ServerConfig {
		std::string name;
		int id;
		std::string ip;
		short port;
		int max_conn;
		bool enable_reconnect;

		uint8_t gate_list_conf_path_type;
		std::string gate_list_conf_local_path;
		std::string gate_list_conf_remote_path;
		std::string log_conf_path;
	};

	const ServerConfig& getServerConfig() { return config_;  }

private:
	rapidjson::Document doc_;
	ServerConfig config_;
	std::string jsonpath_;
};
