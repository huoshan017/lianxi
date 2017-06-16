#pragma once

#include "boost/asio.hpp"
#include "../libjmy/jmy_tcp_client.h"

class GameServer
{
public:
	GameServer();
	~GameServer();

	bool init();
	void clear();
	int run();
	void exit();

private:
	boost::asio::io_service service_;
	JmyTcpClientMaster client_master_;
	JmyTcpClient* gate_client_;
	JmyTcpClient* db_client_;
	bool run_;
};
