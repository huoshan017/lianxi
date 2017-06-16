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

	int start();
	int run();

	void postExitEvent();
	void postConnectGameEvent(const char* ip, unsigned short port);

	JmyTcpClient* getLoginClient() { return login_client_; }
	JmyTcpClient* getGameClient() { return game_client_; }
	void setAccount(const std::string& account) { account_ = account; }
	const std::string& getAccount() const { return account_; }
	void setEnterSession(const std::string& session) { enter_session_ = session; }
	void setReconnSession(const std::string& session) { reconn_session_ = session; }
	const std::string& getEnterSession() const { return enter_session_; }
	const std::string& getReconnSession() const { return reconn_session_; }

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
	std::string account_;
	std::string enter_session_;
	std::string reconn_session_;
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

	bool getAccountByConnId(int conn_id, std::string& account) {
		std::unordered_map<int, std::string>::iterator it = conn_id2accounts_.find(conn_id);
		if (it == conn_id2accounts_.end())
			return false;
		account = it->second;
		return true;
	}

	TestClient* getClientByConnId(int conn_id) {
		std::unordered_map<int, std::string>::iterator it = conn_id2accounts_.find(conn_id);
		if (it == conn_id2accounts_.end())
			return nullptr;

		std::unordered_map<std::string, TestClient*>::iterator tit = clients_.find(it->second);
		if (tit == clients_.end())
			return nullptr;

		return tit->second;
	}

private:
	boost::asio::io_service service_;
	std::unordered_map<std::string, TestClient*> clients_;
	JmyTcpClientMaster client_master_;
	std::unordered_map<int, std::string> conn_id2accounts_;
};

#define CLIENT_MGR (TestClientManager::getInstance())
