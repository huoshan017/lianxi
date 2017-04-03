#pragma once

#include "../thirdparty/include/rapidjson/document.h"
#include "../thirdparty/include/rapidjson/stringbuffer.h"
#include "../thirdparty/include/rapidjson/writer.h"
#include <string>

class ConfigLoader
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
		short port;
		int max_conn;
		bool enable_reconnect;

		std::string listen_game_ip;
		short listen_game_port;
		int listen_game_max_conn;
		bool listen_game_enable_reconnect;

		std::string connect_login_ip;
		short connect_login_port;
		bool connect_login_enable_reconnect;
	};

	const ServerConfig& getServerConfig() { return config_;  }

private:
	rapidjson::Document doc_;
	ServerConfig config_;
	std::string jsonpath_;
};
