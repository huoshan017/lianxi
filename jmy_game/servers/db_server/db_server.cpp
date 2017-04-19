#include "db_server.h"
#include "../common/util.h"
#include "config_loader.h"
#include "config_data.h"
#include "game_handler.h"

DBServer::DBServer() : server_(service_)
{
}

DBServer::~DBServer()
{
}

bool DBServer::init(const char* confpath)
{
	if (!CONFIG_LOADER->loadJson(confpath)) {
		LogError("failed to load server config %s", confpath);
		return false;
	}

	if (!global_log_init(SERVER_CONFIG.log_conf_path.c_str())) {
		LogError("failed to init log with path %s", SERVER_CONFIG.log_conf_path.c_str());
		return false;
	}

	if (!GameHandler::init()) {
		LogError("failed to init GameHandler");
		return false;
	}

	boost::asio::io_service service;
	// listen login and gate
	s_game_config.max_conn = SERVER_CONFIG.max_conn;
	s_game_config.listen_port = SERVER_CONFIG.port;
	s_game_config.listen_ip = (char*)(SERVER_CONFIG.ip.c_str());
	if (!server_.loadConfig(s_game_config)) {
		LogError("failed to load login config");
		return false;
	}

	if (server_.listenStart(s_game_config.listen_port) < 0) {
		LogError("main server listen port %d failed", s_game_config.listen_port);
		return false;
	}

	LogInfo("start listening port %d", s_game_config.listen_port);
	return true;
}

void DBServer::close()
{
	GameHandler::clear();
	server_.close();
}

int DBServer::run()
{
	while (server_.run() >= 0) {
		service_.poll();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	return 0;
}
