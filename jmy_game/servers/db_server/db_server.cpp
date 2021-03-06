#include "db_server.h"
#include "../common/util.h"
#include "config_loader.h"
#include "config_data.h"
#include "db_tables_define.h"
#include "global_data.h"

DBServer::DBServer() : server_(service_), db_mgr_(nullptr)
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

	db_mgr_ = new MysqlDBManager();
	if (!db_mgr_->init(SERVER_CONFIG.mysql_host, SERVER_CONFIG.mysql_user, SERVER_CONFIG.mysql_password, s_jmy_game_db_config)) {
		LogError("failed to init mysql_db_manager");
		return false;
	}

	if (!GLOBAL_DATA->init()) {
		LogError("failed to init global_data");
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

	if (server_.listenStart(s_game_config.listen_ip, s_game_config.listen_port) < 0) {
		LogError("main server listen port %d failed", s_game_config.listen_port);
		return false;
	}

	LogInfo("start listening port %d", s_game_config.listen_port);
	return true;
}

void DBServer::close()
{
	server_.close();
	db_mgr_->clear();
	delete db_mgr_;
	GLOBAL_DATA->clear();
}

int DBServer::run()
{
	while (server_.run() >= 0) {
		if (db_mgr_->run() < 0) {break;}
		GLOBAL_DATA->run();
		service_.poll();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	return 0;
}
