#pragma once

#include "boost/asio.hpp"
#include "../libjmy/jmy_tcp_server.h"
#include "../libjmy/jmy_singleton.hpp"
#include "mysql_db_manager.h"

class DBServer : public JmySingleton<DBServer>
{
public:
	DBServer();
	~DBServer();
	bool init(const char* confpath);
	void close();
	int run();

	MysqlDBManager& getDBMgr() { return db_mgr_; }

private:
	boost::asio::io_service service_;
	JmyTcpServer server_;
	MysqlDBManager db_mgr_;
};

#define DB_SERVER (DBServer::getInstance())
#define DB_MGR (DB_SERVER->getDBMgr())
