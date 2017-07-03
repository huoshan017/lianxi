#pragma once

#define USE_THREAD 1

#include <list>
#include <thread>
#include <boost/asio.hpp>
#if USE_THREAD
#include <boost/lockfree/queue.hpp>
#endif
#include "jmy_tcp_connection.h"
#include "jmy_session_buffer_pool.h"
#if USE_NET_PROTO2
#include "jmy_data_handler2.h"
#else
#include "jmy_data_handler.h"
#endif
#include "jmy_datatype.h"
#include "jmy_util.h"

using namespace boost::asio;
#if USE_THREAD
using namespace boost::lockfree;
#endif

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
#if USE_THREAD
	int do_accept_loop();
	int do_loop();
#else
	int do_accept();
#endif
	int accept_new(ip::tcp::socket* sock);
	int run_conns();

private:
	io_service& service_;
	ip::tcp::socket sock_;
#if USE_THREAD
	std::shared_ptr<std::thread> thread_;
#endif
	std::shared_ptr<ip::tcp::acceptor> acceptor_;
#if USE_NET_PROTO2
	std::shared_ptr<JmyDataHandler2> handler_;
#else
	std::shared_ptr<JmyDataHandler> handler_;
#endif
	std::shared_ptr<JmyEventHandlerManager> event_handler_;
	JmyTcpConnectionMgr conn_mgr_;
	JmyConnectionBufferMgr buffer_mgr_;
	JmyIdGenerator<int> id_gene_;
	JmyTcpConnection curr_conn_;
	std::list<JmyTcpConnection*> conns_;
	std::shared_ptr<JmySessionBufferPool> buff_pool_;
	JmyServerConfig conf_;
	bool inited_;
#if USE_THREAD
	queue<ip::tcp::socket*> free_sock_list_;
	queue<ip::tcp::socket*> accept_sock_list_;
#endif
};
