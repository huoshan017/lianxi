#pragma once

#include "../thirdparty/include/rapidjson/document.h"
#include "../thirdparty/include/rapidjson/stringbuffer.h"
#include "../thirdparty/include/rapidjson/writer.h"
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
		unsigned short port;
		int max_conn;
		bool enable_reconnect;

		std::string connect_gate_ip;
		unsigned short connect_gate_port;
		bool connect_gate_enable_reconnect;

		std::string log_conf_path;
	};

	const ServerConfig& getServerConfig() const { return config_; }

private:
	rapidjson::Document doc_;
	ServerConfig config_;
	std::string jsonpath_;
};

#define CONFIG_LOADER (ConfigLoader::getInstance())
#define SERVER_CONFIG (CONFIG_LOADER->getServerConfig())
