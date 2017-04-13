#pragma once

#include "boost/asio.hpp"
#include "../libjmy/jmy_tcp_server.h"
#include "../libjmy/jmy_singleton.hpp"
#include "mysql_connector_pool.h"

class DBServer : public JmySingleton<DBServer>
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

#define DB_SERVER (DBServer::getInstance())
