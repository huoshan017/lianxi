#pragma once

#include "../libjmy/thirdparty/include/rapidjson/document.h"
#include "../libjmy/thirdparty/include/rapidjson/stringbuffer.h"
#include "../libjmy/thirdparty/include/rapidjson/writer.h"

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

		std::string listen_gate_ip;
		short listen_gate_port;
		int listen_gate_max_conn;
		bool listen_gate_enable_reconnect;
	};

	const ServerConfig& getServerConfig() { return config_;  }

private:
	rapidjson::Document doc_;
	ServerConfig config_;
	std::string jsonpath_;
};
