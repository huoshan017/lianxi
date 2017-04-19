#pragma once

#include "boost/asio.hpp"
#include "../libjmy/jmy_tcp_client.h"
#include "../libjmy/jmy_singleton.hpp"
#include "user_event.h"

class TestClient : public JmySingleton<TestClient>
{
public:
	TestClient();
	~TestClient();
	bool init(const char* confpath);
	void close();
	int run();

	void postExitEvent();
	void postConnectGameEvent(const char* ip, unsigned short port);

private:
	int do_events();
	bool connect_game(const char* ip, unsigned short port);

private:
	boost::asio::io_service service_;
	JmyTcpClient* login_client_;
	JmyTcpClient* game_client_;
	JmyTcpClientMaster client_master_;
	enum State {
		InNone = 0,
		InLogin = 1,
		InGame = 2,
	} state_;
	bool exit_;
	UserEventList event_list_;
};

#define TEST_CLIENT (TestClient::getInstance())
