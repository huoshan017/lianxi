#pragma once

#include "../libjmy/jmy_singleton.hpp"
#include "../thirdparty/include/rapidjson/document.h"
#include "../thirdparty/include/rapidjson/stringbuffer.h"
#include "../thirdparty/include/rapidjson/writer.h"
#include <string>

class ConfigLoader : public JmySingleton<ConfigLoader>
{
public:
	ConfigLoader();
	~ConfigLoader();

	bool loadJson(const char* jsonpath);
	bool reload();
	void clear();

	struct ServerConfig {
		std::string name;
		int id;
		std::string ip;
		unsigned short port;
		int max_conn;
		bool enable_reconnect;

		std::string listen_game_ip;
		unsigned short listen_game_port;
		int listen_game_max_conn;
		bool listen_game_enable_reconnect;

		std::string connect_config_ip;
		unsigned short connect_config_port;
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
#define CONFIG_FILE (CONFIG_LOADER->getServerConfig())
