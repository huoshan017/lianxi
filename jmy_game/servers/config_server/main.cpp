#include "../libjmy/jmy_tcp_server.h"
#include "../libjmy/jmy_tcp_connection.h"
#include "../libjmy/jmy_log.h"
#include "../common/util.h"
#include <iostream>
#include "config_loader.h"
#include "config_data.h"

#define ServerConfPath "./config_server.json"

int main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;

	ConfigLoader config_loader;
	if (!config_loader.loadJson(ServerConfPath)) {
		std::cout << "failed to load server config " << ServerConfPath << std::endl;
		return -1;
	}

	const ConfigLoader::ServerConfig& config = config_loader.getServerConfig();
	const char* log_conf = config.log_conf_path.c_str();
	if (!JmyLogInit(log_conf)) {
		std::cout << "failed to init log file: " << log_conf << std::endl;
		return -1;
	}
	if (!JmyLogOpenLib(s_libjmy_log_cate)) {
		std::cout << "failed to create lib log category: " << s_libjmy_log_cate << std::endl;
		return -1;
	}
	if (!JmyLogOpen(s_server_log_cate)) {
		std::cout << "failed to create server log category: " << s_server_log_cate << std::endl;
		return -1;
	}

	JmyTcpServer main_server;
	s_conn_config.max_conn = config.max_conn;
	s_conn_config.listen_port = config.port;
	s_conn_config.listen_ip = (char*)(config.ip.c_str());
	if (!main_server.loadConfig(s_conn_config)) {
		ServerLogError("failed to load login config");
		return -1;
	}
	if (main_server.listenStart(s_conn_config.listen_port) < 0) {
		ServerLogError("main server listen port %d failed", s_conn_config.listen_port);
		return -1;
	}

	ServerLogInfo("start listening port %d", s_conn_config.listen_port);

	while (main_server.run() >= 0) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	main_server.close();
	return 0;
}
