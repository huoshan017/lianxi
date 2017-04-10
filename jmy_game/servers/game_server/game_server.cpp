#include "game_server.h"
#include "../common/util.h"
#include "config_data.h"
#include "config_loader.h"
#include <thread>

static const char* ServerConfPath = "./game_server.json";

GameServer::GameServer() : client_master_(service_), gate_client_(nullptr)
{
}

GameServer::~GameServer()
{
	clear();
}

bool GameServer::init()
{
	if (!CONFIG_LOADER->loadJson(ServerConfPath)) {
		std::cout << "failed to load server config " << ServerConfPath << std::endl;
		return false;
	}

	if (!global_log_init(SERVER_CONFIG.log_conf_path.c_str())) {
		std::cout << "failed to init log with path " << SERVER_CONFIG.log_conf_path.c_str() << std::endl;
		return false;
	}

	if (!client_master_.init(10)) {
		ServerLogError("client_master init with size(%d) failed", 10);
		return false;
	}
	gate_client_ = client_master_.generate();
	if (!gate_client_) {
		ServerLogError("client_master generate client failed");
		return false;
	}

	s_gate_config.conn_ip = const_cast<char*>(SERVER_CONFIG.connect_gate_ip.c_str());
	s_gate_config.conn_port = SERVER_CONFIG.connect_gate_port;
	if (!gate_client_->start(s_gate_config)) {
		ServerLogError("start connect gate failed");
		return false;
	}
	return true;
}

void GameServer::clear()
{
	if (gate_client_) {
		client_master_.recycle(gate_client_);
		gate_client_ = nullptr;
	}
	client_master_.close();
}

int GameServer::run()
{
	run_  = true;
	int res = 0;
	while (run_) {
		res = gate_client_->run();
		if (res < 0) {
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	return res;
}

void GameServer::exit()
{
	run_ = false;
}
