#pragma once

#include <list>
#include <thread>
#include <boost/asio.hpp>
#if USE_CONNECTOR_AND_SESSION
#include "jmy_tcp_session.h"
#else
#include "jmy_tcp_connection.h"
#endif
#include "jmy_session_buffer_pool.h"
#include "jmy_data_handler.h"
#include "jmy_datatype.h"
#include "jmy_util.h"

using namespace boost::asio;

#define USE_THREAD 0

class JmyTcpServer
{
public:
	JmyTcpServer(io_service& service);
	JmyTcpServer(io_service& service, unsigned short port);
	~JmyTcpServer();

	bool loadConfig(const JmyServerConfig& conf);
	void close();
	int listenStart(const std::string& ip, unsigned short port);
	int listenStart(const char* ip, unsigned short short_port);
	int start();
	int run();

	io_service& getService() { return service_; }
	const JmyServerConfig& getConfig() const { return conf_; }

private:
	int do_accept();
	int accept_new();
#if USE_THREAD
	int do_loop();
#endif
	int run_conns();

private:
	io_service& service_;
	ip::tcp::socket sock_;
#if USE_THREAD
	std::shared_ptr<std::thread> thread_;
#endif
	std::shared_ptr<ip::tcp::acceptor> acceptor_;
	std::shared_ptr<JmyDataHandler> handler_;
	std::shared_ptr<JmyEventHandlerManager> event_handler_;
#if USE_CONNECTOR_AND_SESSION
	std::shared_ptr<JmyTcpSessionMgr> session_mgr_;
	JmyTcpSession curr_session_;
#else
	JmyTcpConnectionMgr conn_mgr_;
	JmyConnectionBufferMgr buffer_mgr_;
	JmyIdGenerator<int> id_gene_;
	JmyTcpConnection curr_conn_;
	std::list<JmyTcpConnection*> conns_;
#endif
	std::shared_ptr<JmySessionBufferPool> buff_pool_;
	JmyServerConfig conf_;
	bool inited_;
};
