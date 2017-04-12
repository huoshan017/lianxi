#pragma once

#include <boost/asio.hpp>
#include "config_loader.h"
#include "../libjmy/jmy_tcp_server.h"
#include "../libjmy/jmy_tcp_client.h"
#include "../libjmy/jmy_tcp_client_set.h"
#include "../libjmy/jmy_singleton.hpp"

class GateServer : public JmySingleton<GateServer>
{
public:
	GateServer();
	~GateServer();

	bool init(const char* conf_path);
	void close();
	bool startLoginClient(const char* ip, unsigned short port);
	bool checkClientMaxCount(int curr_count);
	bool checkGameMaxCount(int curr_count);

	int run();

private:
	boost::asio::io_service service_;
	JmyTcpServer main_server_;			// listen client server
	JmyTcpServer listen_game_server_;	// listen game server
	JmyTcpClientMaster client_master_;
	JmyTcpClient* config_client_;
	JmyTcpClientSet login_client_set_;
};

#define GATE_SERVER (GateServer::getInstance())
