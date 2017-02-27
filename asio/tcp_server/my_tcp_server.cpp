#include "my_tcp_server.h"
#include "my_tcp_session.h"
#include "my_session_buffer_pool.h"
#include <iostream>

MyTcpServer::MyTcpServer() : sock_(service_), curr_session_(NULL), inited_(false)
{
	acceptor_ = std::make_shared<ip::tcp::acceptor>(service_);
	handler_ = std::make_shared<MyDataHandler>();
}

MyTcpServer::MyTcpServer(short port) : sock_(service_), curr_session_(NULL), inited_(false)
{
	acceptor_ = std::make_shared<ip::tcp::acceptor>(service_, ip::tcp::endpoint(ip::tcp::v4(), port));
	handler_ = std::make_shared<MyDataHandler>();
}

MyTcpServer::~MyTcpServer()
{
	close();
}

bool MyTcpServer::loadConfig(const MyServerConfig& conf)
{
	conf_ = conf;
	bool res = handler_->loadMsgHandle(conf.handlers, conf.nhandlers);
	if (!res) return false;
	res = SESSION_MGR->init(conf.max_conn, service_);
	if (res) inited_ = true;
	res = BUFFER_POOL->init(conf.max_conn);
	return res;
}

void MyTcpServer::close()
{
	acceptor_->close();
	thread_->join();
}

int MyTcpServer::start()
{
	if (!inited_) return -1;
	int res = do_accept();
	thread_ = std::make_shared<std::thread>(std::thread(std::bind(&MyTcpServer::do_loop, this)));
	return res;
}

int MyTcpServer::listenStart(short port)
{
	if (!inited_) return -1;
	ip::tcp::endpoint ep(ip::tcp::v4(), port);
	acceptor_->open(ep.protocol());
	acceptor_->set_option(socket_base::reuse_address(true));
	acceptor_->bind(ep);
	acceptor_->listen();
	return start();
}

int MyTcpServer::do_accept()
{
	if (!inited_) return -1;
	if (!curr_session_)
		curr_session_ = SESSION_MGR->getOneSession(conf_.session_conf, handler_);
	if (!curr_session_) {
		std::cout << "get free MyTcpSession failed" << std::endl;
		return -1;
	}
	acceptor_->async_accept(curr_session_->getSock(),
		[this](boost::system::error_code ec){
			if (ec) {
				if (ec.value()==boost::system::errc::operation_canceled || ec.value()==boost::system::errc::operation_in_progress) {
					std::cout << "MyTcpServer do_accept  error code(" << ec.value() << ")" << std::endl;
				} else {
					SESSION_MGR->freeSession(curr_session_);
					std::cout << "MyTcpServer async_accept error: " << ec << std::endl;
					return;	
				}
			} else {
				curr_session_->start();
				curr_session_ = NULL;
				std::cout << "MyTcpServer do_accept  session start" << std::endl;
			}
			do_accept();
		});
	return 1;
}

int MyTcpServer::do_loop()
{
	size_t s = service_.run();
	std::cout << "MyTcpServer::do_loop return result: " << s << std::endl;
	return s;
}

int MyTcpServer::run()
{
	if (SESSION_MGR->run() < 0)
		return -1;
	return 0;
}
