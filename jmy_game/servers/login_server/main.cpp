#include "../libjmy/jmy_tcp_server.h"
#include "../libjmy/jmy_tcp_client.h"
#include "../libjmy/jmy_tcp_client_set.h"
#include "../libjmy/jmy_log.h"
#include "../common/util.h"
#include <iostream>
#include "config_loader.h"
#include "config_data.h"
#include "gate_handler.h"

#define ServerConfPath "./login_server.json"

int main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;

	if (!CONFIG_LOADER->loadJson(ServerConfPath)) {
		ServerLogError("failed to load server config %s", ServerConfPath);
		return -1;
	}

	if (!global_log_init(SERVER_CONFIG.log_conf_path.c_str())) {
		ServerLogError("failed to init log with path %s", SERVER_CONFIG.log_conf_path.c_str());
		return -1;
	}

	boost::asio::io_service service;

	// listen client 
	JmyTcpServer main_server(service);
	s_client_config.max_conn = SERVER_CONFIG.max_conn;
	s_client_config.listen_port = SERVER_CONFIG.port;
	s_client_config.listen_ip = (char*)(SERVER_CONFIG.ip.c_str());
	if (!main_server.loadConfig(s_client_config)) {
		ServerLogError("failed to load login config");
		return -1;
	}
	if (main_server.listenStart(s_client_config.listen_port) < 0) {
		ServerLogError("main server listen port %d failed", s_client_config.listen_port);
		return -1;
	}
	ServerLogInfo("start listening port %d for client", s_client_config.listen_port);

	// listen gate server
	JmyTcpServer listen_gate_server(service);
	s_gate_config.max_conn = SERVER_CONFIG.listen_gate_max_conn;
	s_gate_config.listen_port = SERVER_CONFIG.listen_gate_port;
	s_gate_config.listen_ip = const_cast<char*>(SERVER_CONFIG.listen_gate_ip.c_str());
	if (!listen_gate_server.loadConfig(s_gate_config)) {
		ServerLogError("failed to load listen gate config");
		return -1;
	}
	if (listen_gate_server.listenStart(s_gate_config.listen_port) < 0) {
		ServerLogError("listen port %d for gate server failed", s_gate_config.listen_port);
		return -1;
	}
	GateHandler::init();
	ServerLogInfo("start listening port %d for gate server", s_gate_config.listen_port);

	// connect config server
	JmyTcpClientMaster client_master(main_server.getService());
	if (!client_master.init(2)) {
		ServerLogError("client master init failed");
		return -1;
	}
	JmyTcpClient* config_client = client_master.generate();
	if (!config_client) {
		ServerLogError("generate client to connect config server failed");
		return -1;
	}
	config_client->setIP((char*)SERVER_CONFIG.connect_config_ip.c_str(), SERVER_CONFIG.connect_config_port);
	if (!config_client->start(s_conn_config)) {
		ServerLogError("start client to connect config server failed");
		return -1;
	}

	while (main_server.run() >= 0) {
		listen_gate_server.run();
		config_client->run();
		service.poll();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	client_master.recycle(config_client);
	client_master.close();
	listen_gate_server.close();
	main_server.close();
	return 0;
}
