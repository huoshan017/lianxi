#pragma once

#include <thread>
#include <boost/asio.hpp>
#include "jmy_tcp_session.h"
#include "jmy_session_buffer_pool.h"
#include "jmy_data_handler.h"
#include "jmy_datatype.h"

using namespace boost::asio;

#define USE_THREAD 0

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

	io_service& getService() { return service_; }

private:
	int do_accept();
#if USE_THREAD
	int do_loop();
#endif

private:
	io_service service_;
	ip::tcp::socket sock_;
#if USE_THREAD
	std::shared_ptr<std::thread> thread_;
#endif
	std::shared_ptr<ip::tcp::acceptor> acceptor_;
	std::shared_ptr<JmyDataHandler> handler_;
	std::shared_ptr<JmyTcpSessionMgr> session_mgr_;
	std::shared_ptr<JmySessionBufferPool> session_buff_pool_;
	JmyTcpSession curr_session_;
	JmyServerConfig conf_;
	bool inited_;
};
