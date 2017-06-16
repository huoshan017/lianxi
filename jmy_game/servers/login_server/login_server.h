#pragma once

#include "../libjmy/jmy_tcp_server.h"
#include "../libjmy/jmy_tcp_client.h"
#include "../libjmy/jmy_tcp_client_set.h"
#include "boost/asio.hpp"

class LoginServer
{
public:
	LoginServer();
	~LoginServer();

	bool init(const char* conf_path);
	void clear();

	int run();

private:
	boost::asio::io_service service_;
	JmyTcpServer main_server_;
	JmyTcpServer listen_gate_server_;
	JmyTcpClientMaster client_master_;
	JmyTcpClient* config_client_;
};
