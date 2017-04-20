#include "login_server.h"
#include "config_loader.h"
#include "config_data.h"
#include "gate_server_list.h"
#include "../libjmy/jmy_log.h"
#include "../common/util.h"

LoginServer::LoginServer() : main_server_(service_), listen_gate_server_(service_), client_master_(service_), config_client_(nullptr)
{
}

LoginServer::~LoginServer()
{
}

bool LoginServer::init(const char* conf_path)
{
	if (!CONFIG_LOADER->loadJson(conf_path)) {
		LogError("failed to load server config %s", conf_path);
		return false;
	}

	if (!global_log_init(SERVER_CONFIG.log_conf_path.c_str())) {
		LogError("failed to init log with path %s", SERVER_CONFIG.log_conf_path.c_str());
		return false;
	}

	// listen client 
	s_client_config.max_conn = SERVER_CONFIG.max_conn;
	s_client_config.listen_port = SERVER_CONFIG.port;
	s_client_config.listen_ip = (char*)(SERVER_CONFIG.ip.c_str());
	if (!main_server_.loadConfig(s_client_config)) {
		LogError("failed to load login config");
		return -1;
	}
	if (main_server_.listenStart(s_client_config.listen_port) < 0) {
		LogError("main server listen port %d failed", s_client_config.listen_port);
		return -1;
	}
	LogInfo("start listening port %d for client", s_client_config.listen_port);

	// listen gate server
	s_gate_config.max_conn = SERVER_CONFIG.listen_gate_max_conn;
	s_gate_config.listen_port = SERVER_CONFIG.listen_gate_port;
	s_gate_config.listen_ip = const_cast<char*>(SERVER_CONFIG.listen_gate_ip.c_str());
	if (!listen_gate_server_.loadConfig(s_gate_config)) {
		LogError("failed to load listen gate config");
		return -1;
	}
	if (listen_gate_server_.listenStart(s_gate_config.listen_port) < 0) {
		LogError("listen port %d for gate server failed", s_gate_config.listen_port);
		return -1;
	}
	LogInfo("start listening port %d for gate server", s_gate_config.listen_port);

	GATE_SERVER_LIST->init();

	// connect config server
	if (!client_master_.init(2)) {
		LogError("client master init failed");
		return -1;
	}
	config_client_ = client_master_.generate();
	if (!config_client_) {
		LogError("generate client to connect config server failed");
		return -1;
	}
	config_client_->setIP((char*)SERVER_CONFIG.connect_config_ip.c_str(), SERVER_CONFIG.connect_config_port);
	if (!config_client_->start(s_conn_config)) {
		LogError("start client to connect config server failed");
		return -1;
	}
	return true;
}

void LoginServer::clear()
{
	if (config_client_) {
		config_client_->close();
		config_client_ = nullptr;
		client_master_.recycle(config_client_);
	}
	client_master_.close();
	listen_gate_server_.close();
	main_server_.close();
}

int LoginServer::run()
{
	int res = 0;
	while (main_server_.run() >= 0) {
		res = listen_gate_server_.run();
		if (res < 0) break;
		res = config_client_->run();
		if (res < 0) break;
		service_.poll();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	return res;
}
