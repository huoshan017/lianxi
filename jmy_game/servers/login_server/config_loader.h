#pragma once

#include "../libjmy/thirdparty/include/rapidjson/document.h"
#include "../libjmy/thirdparty/include/rapidjson/stringbuffer.h"
#include "../libjmy/thirdparty/include/rapidjson/writer.h"
#include "../libjmy/jmy_singleton.hpp"

class ConfigLoader : public JmySingleton<ConfigLoader>
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

		std::string listen_gate_ip;
		short listen_gate_port;
		int listen_gate_max_conn;
		bool listen_gate_enable_reconnect;

		std::string connect_config_ip;
		short connect_config_port;
		bool connect_config_enable_reconnect;

		std::string log_conf_path;
	};

	const ServerConfig& getServerConfig() { return config_;  }

private:
	rapidjson::Document doc_;
	ServerConfig config_;
	std::string jsonpath_;
};

#define CONFIG_LOADER (ConfigLoader::getInstance())
#define SERVER_CONFIG_FILE (CONFIG_LOADER->getServerConfig())
