#pragma once

#include "../../libjmy/thirdparty/include/rapidjson/document.h"
#include "../../libjmy/thirdparty/include/rapidjson/stringbuffer.h"
#include "../../libjmy/thirdparty/include/rapidjson/writer.h"

class ConfigLoader
{
public:
	ConfigLoader();
	~ConfigLoader();

	bool loadJson(const char* jsonpath);
	void close();

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

		std::string connect_gate_ip;
		short connect_gate_port;
		bool connect_gate_enable_reconnect;
	};

	const ServerConfig& getServerConfig() { return config_;  }

private:
	rapidjson::Document doc_;
	ServerConfig config_;
};
