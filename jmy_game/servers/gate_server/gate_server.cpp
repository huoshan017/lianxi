#include "gate_server.h"
#include "config_data.h"
#include "client_handler.h"
#include "client_manager.h"

GateServer::GateServer() : main_server_(service_), listen_game_server_(service_), client_master_(service_), config_client_(nullptr)
{
}

GateServer::~GateServer()
{
	close();
}

bool GateServer::init(const char* conf_path)
{
	if (!CONFIG_LOADER->loadJson(conf_path)) {
		LogError("failed to load server config %s", conf_path);
		return false;
	}

	if (!global_log_init(CONFIG_FILE.log_conf_path.c_str())) {
		LogError("failed to init log with path %s", CONFIG_FILE.log_conf_path.c_str());
		return false;
	}

	// listen client
	s_client_config.max_conn = CONFIG_FILE.max_conn;
	s_client_config.listen_port = CONFIG_FILE.port;
	s_client_config.listen_ip = (char*)(CONFIG_FILE.ip.c_str());
	if (!main_server_.loadConfig(s_client_config)) {
		LogError("failed to load listen client config");
		return false;
	}
	if (main_server_.listenStart(CONFIG_FILE.ip, CONFIG_FILE.port) < 0) {
		LogError("main server listen port %d failed", CONFIG_FILE.port);
		return false;
	}
	LogInfo("start listening port %d", CONFIG_FILE.port);

	// listen game
	s_game_config.max_conn = CONFIG_FILE.listen_game_max_conn;
	s_game_config.listen_ip = (char*)CONFIG_FILE.listen_game_ip.c_str();
	s_game_config.listen_port = CONFIG_FILE.listen_game_port;
	if (!listen_game_server_.loadConfig(s_game_config)) {
		LogError("failed to load listen game server config");
		return false;
	}
	if (listen_game_server_.listenStart(CONFIG_FILE.listen_game_ip, CONFIG_FILE.listen_game_port) < 0) {
		LogError("listen game server port %d failed", CONFIG_FILE.listen_game_port);
		return false;
	}
	LogInfo("start listening port %d for game server", CONFIG_FILE.listen_game_port);

	if (!client_master_.init(10)) {
		LogError("client master init failed");
		return false;
	}

	// connection to config_server
	config_client_ = client_master_.generate();
	if (!config_client_) {
		LogError("create client for connection to config_server failed");
		return false;
	}

	config_client_->setIP((char*)CONFIG_FILE.connect_config_ip.c_str(), CONFIG_FILE.connect_config_port);
	if (!config_client_->start(s_config_config)) {
		LogError("client for connection to config_server start failed");
		return false;
	}
	LogInfo("start connect to config server(%s:%d)", CONFIG_FILE.connect_config_ip.c_str(), CONFIG_FILE.connect_config_port);

	if (!CLIENT_MANAGER->init()) {
		LogError("client handler init failed");
		return false;
	}

	LogInfo("GateServer inited");
	return true;
}

void GateServer::close()
{
	CLIENT_MANAGER->clear();
	client_master_.close();
	login_client_set_.clear();
	listen_game_server_.close();
	main_server_.close();
}

int GateServer::run()
{
	while (true) {
		if (main_server_.run() < 0)
			break;
		if (listen_game_server_.run() < 0)
			break;
		login_client_set_.run();
		config_client_->run();
		service_.poll();
		std::this_thread::sleep_for(std::chrono::microseconds(100));
	}
	return 0;
}

JmyTcpClient* GateServer::startLoginClient(const char* ip, unsigned short port)
{
	JmyTcpClient* client = client_master_.generate();
	if (!client) {
		LogError("client_master generate new client failed");
		return nullptr;
	}

	client->setIP(const_cast<char*>(ip), port);
	if (!client->start(s_login_config)) {
		LogError("GateServer start client failed");
		return nullptr;
	}
	if (!login_client_set_.addClient(client, &s_login_config)) {
		client->close();
		return nullptr;
	}
	return client;
}

bool GateServer::removeLoginClient(JmyTcpClient* client)
{
	if (!login_client_set_.removeClient(client)) {
		LogError("remove client from login client set failed");
		return false;
	}
	if (!client_master_.recycle(client)) {
		LogError("recycle client failed");
		return false;
	}
	return true;
}

bool GateServer::checkClientMaxCount(int curr_count)
{
	const JmyServerConfig& config = main_server_.getConfig();
	if (curr_count >= (int)config.max_conn) return true;
	return false;
}

bool GateServer::checkGameMaxCount(int curr_count)
{
	const JmyServerConfig& config = listen_game_server_.getConfig();
	if (curr_count >= (int)config.max_conn) return true;
	return false;
}
