#pragma once

#include <boost/asio.hpp>
#include "jmy_data_handler.h"
#include "jmy_datatype.h"
#include <thread>

using namespace boost::asio;

class JmyTcpSession;

class JmyTcpServer
{
public:
	JmyTcpServer();
	JmyTcpServer(short port);
	~JmyTcpServer();

	bool loadConfig(const JmyServerConfig& conf);
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
	std::shared_ptr<JmyDataHandler> handler_;
	JmyTcpSession* curr_session_;
	JmyServerConfig conf_;
	bool inited_;
};
