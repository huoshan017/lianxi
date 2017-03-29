#include "../libjmy/jmy_tcp_server.h"
#include "../libjmy/jmy_log.h"
#include "../common/util.h"
#include <iostream>
#include "config_loader.h"
#include "config_data.h"

#define LogConfPath "./log.conf"
#define ServerConfPath "./login_server.json"

int main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;

	if (!JmyLogInit(LogConfPath)) {
		std::cout << "failed to init log file: " << LogConfPath << std::endl;
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

	ConfigLoader config_loader;
	if (!config_loader.loadJson(ServerConfPath)) {
		ServerLogError("failed to load server config %s", ServerConfPath);
		return -1;
	}

	const ConfigLoader::ServerConfig& config = config_loader.getServerConfig();

	JmyTcpServer main_server;
	s_login_config.max_conn = config.max_conn;
	s_login_config.listen_port = config.port;
	s_login_config.listen_ip = (char*)(config.ip.c_str());
	if (!main_server.loadConfig(s_login_config)) {
		ServerLogError("failed to load login config");
		return -1;
	}

	if (main_server.listenStart(config.port) < 0) {
		ServerLogError("main server listen port %d failed", s_login_config.listen_port);
		return -1;
	}

	ServerLogInfo("start listening port %d", config.port);

	JmyTcpServer listen_gate_server;
	s_gate_config.max_conn = config.listen_gate_max_conn;
	s_gate_config.listen_port = config.listen_gate_port;
	s_gate_config.listen_ip = const_cast<char*>(config.listen_gate_ip.c_str());
	if (!listen_gate_server.loadConfig(s_gate_config)) {
		ServerLogError("failed to load listen gate config");
		return -1;
	}

	if (listen_gate_server.listenStart(s_gate_config.listen_port) < 0) {
		ServerLogError("listen port %d for gate server failed", s_gate_config.listen_port);
		return -1;
	}

	ServerLogInfo("start listening port %d for gate server", s_gate_config.listen_port);

	while (main_server.run() >= 0) {
		listen_gate_server.run();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	listen_gate_server.close();
	main_server.close();
	return 0;
}
