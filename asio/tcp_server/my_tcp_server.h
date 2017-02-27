#pragma once

#include <boost/asio.hpp>
#include "my_data_handler.h"
#include "my_datatype.h"
#include <thread>

using namespace boost::asio;

class MyTcpSession;

class MyTcpServer
{
public:
	MyTcpServer();
	MyTcpServer(short port);
	~MyTcpServer();

	bool loadConfig(const MyServerConfig& conf);
	void close();
	int listenStart(short port);
	int start();
	int run();

private:
	int do_accept();
	int do_loop();

private:
	io_service service_;
	ip::tcp::socket sock_;
	std::shared_ptr<std::thread> thread_;
	std::shared_ptr<ip::tcp::acceptor> acceptor_;
	std::shared_ptr<MyDataHandler> handler_;
	MyTcpSession* curr_session_;
	MyServerConfig conf_;
	bool inited_;
};
