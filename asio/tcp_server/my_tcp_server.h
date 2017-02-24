#pragma once

#include <boost/asio.hpp>
#include "my_data_handler.h"
#include "my_datatype.h"

using namespace boost::asio;

class MyTcpServer
{
public:
	MyTcpServer();
	MyTcpServer(short port);
	~MyTcpServer();

	bool loadConfig(const MyServerConfig& conf);
	void close();
	int listenStart(short port);
	int run();

private:
	int do_accept();

private:
	io_service service_;
	std::shared_ptr<ip::tcp::acceptor> acceptor_;
	std::shared_ptr<MyDataHandler> handler_;
	MyServerConfig conf_;
	bool inited_;
};
