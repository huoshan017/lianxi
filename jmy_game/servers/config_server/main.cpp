#include "../libjmy/jmy_tcp_server.h"
#include "../libjmy/jmy_tcp_connection.h"
#include "../libjmy/jmy_log.h"
#include "../common/util.h"
#include <iostream>
#include "config_loader.h"
#include "conf_gate_list.h"
#include "config_data.h"

#define ServerConfPath "./config_server.json"
#define ConfGateListPath "./server_list.json"

int main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;

	if (!CONFIG_LOADER->loadJson(ServerConfPath)) {
		LogError("failed to load server config %s", ServerConfPath);
		return -1;
	}

	if (!global_log_init(SERVER_CONFIG.log_conf_path.c_str())) {
		LogError("failed to init log with path %s", SERVER_CONFIG.log_conf_path.c_str());
		return -1;
	}

	if (!CONF_GATE_LIST->loadJson(ConfGateListPath)) {
		LogError("failed to load gate_list_conf(%s) file", ConfGateListPath);
		return -1;
	}

	boost::asio::io_service service;
	// listen login and gate
	JmyTcpServer main_server(service);
	s_conn_config.max_conn = SERVER_CONFIG.max_conn;
	s_conn_config.listen_port = SERVER_CONFIG.port;
	s_conn_config.listen_ip = (char*)(SERVER_CONFIG.ip.c_str());
	if (!main_server.loadConfig(s_conn_config)) {
		LogError("failed to load login config");
		return -1;
	}
	if (main_server.listenStart(s_conn_config.listen_port) < 0) {
		LogError("main server listen port %d failed", s_conn_config.listen_port);
		return -1;
	}

	LogInfo("start listening port %d", s_conn_config.listen_port);

	while (main_server.run() >= 0) {
		service.poll();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	main_server.close();
	return 0;
}
