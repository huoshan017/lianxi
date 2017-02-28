#include "jmy_tcp_server.h"
#include "jmy_tcp_session.h"
#include "jmy_session_buffer_pool.h"
#include <iostream>

JmyTcpServer::JmyTcpServer() : sock_(service_), curr_session_(NULL), inited_(false)
{
	acceptor_ = std::make_shared<ip::tcp::acceptor>(service_);
	handler_ = std::make_shared<JmyDataHandler>();
	session_mgr_ = std::make_shared<JmyTcpSessionMgr>();
	session_buff_pool_ = std::make_shared<JmySessionBufferPool>();
}

JmyTcpServer::JmyTcpServer(short port) : sock_(service_), curr_session_(NULL), inited_(false)
{
	acceptor_ = std::make_shared<ip::tcp::acceptor>(service_, ip::tcp::endpoint(ip::tcp::v4(), port));
	handler_ = std::make_shared<JmyDataHandler>();
	session_mgr_ = std::make_shared<JmyTcpSessionMgr>();
	session_buff_pool_ = std::make_shared<JmySessionBufferPool>();
}

JmyTcpServer::~JmyTcpServer()
{
	close();
}

bool JmyTcpServer::loadConfig(const JmyServerConfig& conf)
{
	conf_ = conf;
	bool res = handler_->loadMsgHandle(conf.handlers, conf.nhandlers);
	if (!res) return false;
	res = session_mgr_->init(conf.max_conn, service_);
	if (res) inited_ = true;
	res = session_buff_pool_->init(conf.max_conn,
							conf.session_conf.send_buff_min,
							conf.session_conf.recv_buff_min,
							conf.session_conf.send_buff_max,
							conf.session_conf.recv_buff_max);
	return res;
}

void JmyTcpServer::close()
{
	session_buff_pool_->clear();
	session_mgr_->clear();
	acceptor_->close();
	thread_->join();
}

int JmyTcpServer::start()
{
	if (!inited_) return -1;
	int res = do_accept();
	thread_ = std::make_shared<std::thread>(std::thread(std::bind(&JmyTcpServer::do_loop, this)));
	return res;
}

int JmyTcpServer::listenStart(short port)
{
	if (!inited_) return -1;
	ip::tcp::endpoint ep(ip::tcp::v4(), port);
	acceptor_->open(ep.protocol());
	acceptor_->set_option(socket_base::reuse_address(true));
	acceptor_->bind(ep);
	acceptor_->listen();
	return start();
}

int JmyTcpServer::do_accept()
{
	if (!inited_) return -1;
	if (!curr_session_)
		curr_session_ = session_mgr_->getOneSession(session_buff_pool_, handler_);
	if (!curr_session_) {
		std::cout << "JmyTcpServer::do_accept  get free MyTcpSession failed" << std::endl;
		return -1;
	}
	acceptor_->async_accept(curr_session_->getSock(),
		[this](boost::system::error_code ec){
			if (ec) {
				if (ec.value()==boost::system::errc::operation_canceled || ec.value()==boost::system::errc::operation_in_progress) {
					std::cout << "JmyTcpServer::do_accept  error code(" << ec.value() << ")" << std::endl;
				} else {
					session_mgr_->freeSession(curr_session_);
					std::cout << "JmyTcpServer::do_accept  async_accept error: " << ec << std::endl;
					return;	
				}
			} else {
				curr_session_->start();
				curr_session_ = NULL;
				std::cout << "JmyTcpServer::do_accept  session start" << std::endl;
			}
			do_accept();
		});
	return 1;
}

int JmyTcpServer::do_loop()
{
	size_t s = service_.run();
	std::cout << "JmyTcpServer::do_loop return result: " << s << std::endl;
	return s;
}

int JmyTcpServer::run()
{
	if (session_mgr_->run() < 0)
		return -1;
	return 0;
}
