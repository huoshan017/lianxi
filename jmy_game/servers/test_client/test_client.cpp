#include "test_client.h"
#include "config_loader.h"
#include "config_data.h"
#include "../libjmy/jmy_mem.h"
#include "../common/util.h"
#include <thread>

TestClient::TestClient(JmyTcpClient* login_client, JmyTcpClient* game_client)
: login_client_(login_client), game_client_(game_client), state_(InNone), exit_(false)
{
}

TestClient::~TestClient()
{
	close();
}

void TestClient::close()
{
}

int TestClient::start()
{
	state_ = InLogin;
	return 0;
}

int TestClient::run()
{
	if (!exit_) {
		if (state_ == InLogin) {
			if (login_client_ && login_client_->run() < 0) {
				return -1;
			}
		} else if (state_ == InGame) {
			if (game_client_ && game_client_->run() < 0) {
				return -1;
			}
		} else {
		}
		do_events();
	}
	return 0;
}

void TestClient::postExitEvent()
{
	event_list_.pushEvent(USER_EVENT_EXIT_GAME, nullptr, 0, "");
}

void TestClient::postConnectGameEvent(const char* ip, unsigned short port)
{
	std::string ip_str = ip;
	event_list_.pushEvent(USER_EVENT_CONNECT_GAME_SERVER, nullptr, port, ip_str);
}

bool TestClient::connect_game(const char* ip, unsigned short port)
{
	game_client_->setIP(ip, port);
	if (!game_client_->start(s_game_config)) {
		LogError("connect server(%s:%d) failed", ip, port);
		return false;
	}
	state_ = InGame;
	return true;
}

int TestClient::do_events()
{
	int c = 0;
	UserEvent event;
	while (true) {
		if (!event_list_.popEvent(event)) break;
		switch (event.event_id) {
		case USER_EVENT_CONNECT_GAME_SERVER:
			{
				const char* connect_ip = event.str_param.c_str();
				unsigned short connect_port = event.l_param;	
				if (!connect_game(connect_ip, connect_port)) {
					LogError("connect game(%s:%d) failed", connect_ip, connect_port);
					return -1;
				}
				c += 1;
			}
			break;
		case USER_EVENT_EXIT_GAME:
			{
				exit_ = true;
				c += 1;
			}
			break;
		default:
			{
				LogWarn("event %d not handled", event.event_id);
			}
			break;
		}
	}
	return c;
}

// TestClientManager
TestClientManager::TestClientManager() : client_master_(service_)
{
}

TestClientManager::~TestClientManager()
{
}

bool TestClientManager::init(const char* conf_path)
{
	if (!CONFIG_LOADER->loadJson(conf_path)) {
		std::cout << "failed to load client config " << conf_path << std::endl;
		return false;
	}

	if (!global_log_init(CLIENT_CONFIG.log_conf_path.c_str())) {
		std::cout << "failed to init log with path " << CLIENT_CONFIG.log_conf_path.c_str() << std::endl;
		return false;
	}

	if (!client_master_.init(4*CLIENT_CONFIG.account_num)) {
		LogError("client_master init with size(%d) failed", 10);
		return false;
	}

	return true;
}

void TestClientManager::clear()
{
	client_master_.close();
}

bool TestClientManager::startClient(const std::string& account)
{
	if (clients_.find(account) != clients_.end()) {
		LogInfo("already start client(%s)", account.c_str());
		return true;
	}

	JmyTcpClient* login_client = client_master_.generate();
	if (!login_client) {
		LogError("client_master generate client failed");
		return false;
	}

	login_client->setIP(const_cast<char*>(CLIENT_CONFIG.connect_ip.c_str()), CLIENT_CONFIG.connect_port);
	if (!login_client->start(s_login_config)) {
		LogError("start connect login failed");
		return false;
	}

	conn_id2accounts_.insert(std::make_pair(login_client->getId(), account));
	
	JmyTcpClient* game_client = client_master_.generate();
	if (!game_client) {
		LogError("get new client failed");
		return false;
	}

	conn_id2accounts_.insert(std::make_pair(game_client->getId(), account));

	TestClient* client = jmy_mem_malloc<TestClient>(login_client, game_client);
	client->start();

	clients_.insert(std::make_pair(account, client));
	LogInfo("start client(%s)", account.c_str());
	return true;
}

bool TestClientManager::stopClient(const std::string& account)
{
	std::unordered_map<std::string, TestClient*>::iterator it = clients_.find(account);
	if (it == clients_.end()) {
		LogWarn("not found client(%s)", account.c_str());
		return false;
	}
	if (it->second) {
		JmyTcpClient* client = it->second->getLoginClient();
		client->close();
		conn_id2accounts_.erase(client->getId());
		client = it->second->getGameClient();
		client->close();
		conn_id2accounts_.erase(client->getId());
		jmy_mem_free(it->second);
	}
	clients_.erase(it);
	LogInfo("stop client(%s)", account.c_str());
	return true;
}

int TestClientManager::run()
{
	std::unordered_map<std::string, TestClient*>::iterator it;
	while (true) {
		it = clients_.begin();
		for (; it!=clients_.end(); ++it) {
			if (!it->second) {
				continue;
			}
			it->second->run();
		}
		service_.poll();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	return 0;
}
