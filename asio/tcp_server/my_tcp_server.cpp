#include "my_tcp_server.h"
#include "my_tcp_session.h"
#include <iostream>

MyTcpServer::MyTcpServer() : inited_(false)
{
	acceptor_ = std::make_shared<ip::tcp::acceptor>(service_);
	handler_ = std::make_shared<MyDataHandler>();
}

MyTcpServer::MyTcpServer(short port) : inited_(false)
{
	acceptor_ = std::make_shared<ip::tcp::acceptor>(service_, ip::tcp::endpoint(ip::tcp::v4(), port));
	handler_ = std::make_shared<MyDataHandler>();
}

MyTcpServer::~MyTcpServer()
{
}

bool MyTcpServer::loadConfig(const MyServerConfig& conf)
{
	conf_ = conf;
	bool res = handler_->loadMsgHandle(conf.handlers, conf.nhandlers);
	inited_ = true;
	return res;
}

void MyTcpServer::close()
{
	acceptor_->close();
}

int MyTcpServer::listenStart(short port)
{
	if (!inited_) return -1;
	acceptor_->bind(ip::tcp::endpoint(ip::tcp::v4(), port));
	acceptor_->listen();
	do_accept();
	return 1;
}

int MyTcpServer::do_accept()
{
	if (!inited_) return -1;
	MyTcpSession* session = SESSION_MGR->getOneSession(conf_.session_conf, handler_);
	if (!session) {
		std::cout << "get free MyTcpSession failed" << std::endl;
		return -1;
	}

	acceptor_->async_accept(session->getSock(),
		[this, session](boost::system::error_code ec){
			if (!ec) {
				session->start();
			} else {
				SESSION_MGR->freeSession(session);
				std::cout << "MyTcpServer async_accept error: " << ec << std::endl;
			}
			do_accept();
		});
	return 1;
}

int MyTcpServer::run()
{
	service_.run();
	return 0;
}
