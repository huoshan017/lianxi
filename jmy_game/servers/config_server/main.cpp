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
		ServerLogError("failed to load server config %s", ServerConfPath);
		return -1;
	}

	const ConfigLoader::ServerConfig& config_file = config_loader.getServerConfig();

	if (!global_log_init(config_file.log_conf_path.c_str())) {
		ServerLogError("failed to init log with path %s", config_file.log_conf_path.c_str());
		return -1;
	}

	boost::asio::io_service service;
	// listen login and gate
	JmyTcpServer main_server(service);
	s_conn_config.max_conn = config_file.max_conn;
	s_conn_config.listen_port = config_file.port;
	s_conn_config.listen_ip = (char*)(config_file.ip.c_str());
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
