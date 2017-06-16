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

	struct ClientConfig {
		std::string connect_ip;
		unsigned short connect_port;
		bool enable_reconnect;
		std::string log_conf_path;
		std::string account_prefix;
		int account_start_index;
		int account_num;
	};

	const ClientConfig& getClientConfig() const { return config_; }

private:
	rapidjson::Document doc_;
	ClientConfig config_;
	std::string jsonpath_;
};

#define CONFIG_LOADER (ConfigLoader::getInstance())
#define CLIENT_CONFIG (CONFIG_LOADER->getClientConfig())
