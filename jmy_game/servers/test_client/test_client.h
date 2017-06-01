#pragma once

#include <unordered_map>
#include "boost/asio.hpp"
#include "../libjmy/jmy_tcp_client.h"
#include "../libjmy/jmy_singleton.hpp"
#include "user_event.h"

class TestClient
{
public:
	TestClient(JmyTcpClient* login_client, JmyTcpClient* game_client);
	~TestClient();
	void close();
	int run();

	void postExitEvent();
	void postConnectGameEvent(const char* ip, unsigned short port);

private:
	int do_events();
	bool connect_game(const char* ip, unsigned short port);

private:
	JmyTcpClient* login_client_;
	JmyTcpClient* game_client_;
	enum State {
		InNone = 0,
		InLogin = 1,
		InGame = 2,
	} state_;
	bool exit_;
	UserEventList event_list_;
};

class TestClientManager : public JmySingleton<TestClientManager>
{
public:
	TestClientManager();
	~TestClientManager();

	bool init(const char* conf_path);
	void clear();

	bool startClient(const std::string& account);
	bool stopClient(const std::string& account);

	int run();

private:
	boost::asio::io_service service_;
	std::unordered_map<std::string, TestClient*> clients_;
	JmyTcpClientMaster client_master_;
};

#define CLIENT_MGR (TestClientManager::getInstance())
