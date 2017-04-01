#include "../libjmy/jmy_tcp_server.h"
#include "../libjmy/jmy_tcp_connection.h"
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

	// listen client server
	JmyTcpServer main_server;
	s_client_config.max_conn = config.max_conn;
	s_client_config.listen_port = config.port;
	s_client_config.listen_ip = (char*)(config.ip.c_str());
	if (!main_server.loadConfig(s_client_config)) {
		ServerLogError("failed to load listen client config");
		return -1;
	}
	if (main_server.listenStart(config.port) < 0) {
		ServerLogError("main server listen port %d failed", config.port);
		return -1;
	}
	ServerLogInfo("start listening port %d", config.port);

	// listen game server
	JmyTcpServer listen_game_server;
	s_game_config.max_conn = config.listen_game_max_conn;
	s_game_config.listen_ip = (char*)config.listen_game_ip.c_str();
	s_game_config.listen_port = config.listen_game_port;
	if (!listen_game_server.loadConfig(s_game_config)) {
		ServerLogError("failed to load listen game server config");
		return -1;
	}

	// connection to login server
	JmyTcpConnectionMgr conn_mgr(main_server.getService());
	JmyTcpConnection* login_conn = conn_mgr.getFree(config.id);
	if (!login_conn) {
		ServerLogError("get free connection with id(%d) failed", config.id);
		return -1;
	}
	login_conn->asynConnect(config.connect_login_ip.c_str(), config.connect_login_port);
	while (!login_conn->isConnected()) {
		std::this_thread::sleep_for(std::chrono::microseconds(100));
	}

	while (main_server.run() >= 0) {
		if (login_conn->isConnected()) {
			if (login_conn->run() < 0) {
				ServerLogInfo("login connection run failed");
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	main_server.close();
	return 0;
}
