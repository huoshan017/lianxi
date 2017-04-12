#pragma once

#include "boost/asio.hpp"
#include "../libjmy/jmy_tcp_server.h"

class DBServer
{
public:
	DBServer();
	~DBServer();
	bool init(const char* confpath);
	void close();
	int run();

private:
	boost::asio::io_service service_;
	JmyTcpServer server_;
};
