#include "gate_server.h"
#include "config_data.h"

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
		ServerLogError("failed to load server config %s", conf_path);
		return false;
	}

	if (!global_log_init(CONFIG_FILE.log_conf_path.c_str())) {
		ServerLogError("failed to init log with path %s", CONFIG_FILE.log_conf_path.c_str());
		return false;
	}

	s_client_config.max_conn = CONFIG_FILE.max_conn;
	s_client_config.listen_port = CONFIG_FILE.port;
	s_client_config.listen_ip = (char*)(CONFIG_FILE.ip.c_str());
	if (!main_server_.loadConfig(s_client_config)) {
		ServerLogError("failed to load listen client config");
		return false;
	}
	if (main_server_.listenStart(CONFIG_FILE.port) < 0) {
		ServerLogError("main server listen port %d failed", CONFIG_FILE.port);
		return false;
	}
	ServerLogInfo("start listening port %d", CONFIG_FILE.port);

	s_game_config.max_conn = CONFIG_FILE.listen_game_max_conn;
	s_game_config.listen_ip = (char*)CONFIG_FILE.listen_game_ip.c_str();
	s_game_config.listen_port = CONFIG_FILE.listen_game_port;
	if (!listen_game_server_.loadConfig(s_game_config)) {
		ServerLogError("failed to load listen game server config");
		return false;
	}
	if (listen_game_server_.listenStart(CONFIG_FILE.listen_game_port) < 0) {
		ServerLogError("listen game server port %d failed", CONFIG_FILE.listen_game_port);
		return false;
	}

	if (!client_master_.init(10)) {
		ServerLogError("client master init failed");
		return false;
	}

	// connection to config_server
	config_client_ = client_master_.generate();
	if (!config_client_) {
		ServerLogError("create client for connection to config_server failed");
		return false;
	}
	s_config_config.conn_ip = (char*)CONFIG_FILE.connect_config_ip.c_str();
	s_config_config.conn_port = CONFIG_FILE.connect_config_port;
	if (!config_client_->start(s_config_config)) {
		ServerLogError("client for connection to config_server start failed");
		return false;
	}

	ServerLogInfo("GateServer inited");
	return true;
}

void GateServer::close()
{
	client_master_.close();
	login_client_set_.clear();
	listen_game_server_.close();
	main_server_.close();
}

int GateServer::run()
{
	while (main_server_.run() >= 0) {
		listen_game_server_.run();
		login_client_set_.run();
		config_client_->run();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	return 0;
}

bool GateServer::startLoginClient()
{
	JmyTcpClient* client = client_master_.generate();
	if (!client) {
		ServerLogError("client_master generate new client failed");
		return false;
	}
	if (!client->start(s_login_config)) {
		ServerLogError("GateServer start client failed");
		return false;
	}
	if (!login_client_set_.addClient(client, &s_login_config)) {
		client->close();
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