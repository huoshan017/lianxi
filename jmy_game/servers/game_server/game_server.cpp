#include "game_server.h"
#include "../common/util.h"
#include "config_data.h"
#include "config_loader.h"
#include <thread>
#include "gm.h"
#include "gm_cmd.h"

static const char* ServerConfPath = "./game_server.json";

GameServer::GameServer() : client_master_(service_), gate_client_(nullptr), db_client_(nullptr)
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
		LogError("client_master init with size(%d) failed", 10);
		return false;
	}

	// gate client
	gate_client_ = client_master_.generate();
	if (!gate_client_) {
		LogError("client_master generate gate client failed");
		return false;
	}
	gate_client_->setIP(const_cast<char*>(SERVER_CONFIG.connect_gate_ip.c_str()), SERVER_CONFIG.connect_gate_port);
	if (!gate_client_->start(s_gate_config)) {
		LogError("start connect gate_server failed");
		return false;
	}

	// db client
	db_client_ = client_master_.generate();
	if (!db_client_) {
		LogError("client_master generate db client failed");
		return false;
	}
	const char* db_ip = SERVER_CONFIG.connect_db_ip.c_str();
	unsigned short db_port = SERVER_CONFIG.connect_db_port;
	db_client_->setIP(db_ip, db_port);
	if (!db_client_->start(s_db_config)) {
		LogError("start connect db_server failed");
		return false;
	}

	GM_MGR->load_cmds(gm_cmds, sizeof(gm_cmds)/sizeof(gm_cmds[0]));

	return true;
}

void GameServer::clear()
{
	if (gate_client_) {
		client_master_.recycle(gate_client_);
		gate_client_ = nullptr;
	}
	if (db_client_) {
		client_master_.recycle(db_client_);
		db_client_ = nullptr;
	}
	client_master_.close();
}

int GameServer::run()
{
	run_  = true;
	int res = 0;
	while (run_) {
		res = gate_client_->run();
		if (res < 0) break;
		res = db_client_->run();
		if (res < 0) break;
		service_.poll();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	return res;
}

void GameServer::exit()
{
	run_ = false;
}
