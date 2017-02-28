#include "jmy_tcp_connector.h"
#include <iostream>

JmyTcpConnector::JmyTcpConnector()
{
}

JmyTcpConnector::JmyTcpConnector(io_service& service, const ip::tcp::endpoint& ep)
	: ep_(ep)
{
	sock_ = std::make_shared<ip::tcp::socket>(service);
}

JmyTcpConnector::~JmyTcpConnector()
{
	destroy();
}

bool JmyTcpConnector::init(io_service& service)
{
	sock_ = std::make_shared<ip::tcp::socket>(service);
	return true;
}

void JmyTcpConnector::close()
{
	sock_->close();
}

void JmyTcpConnector::destroy()
{
	close();
}

bool JmyTcpConnector::loadConfig(const JmyConnectorConfig& conf)
{
	if (!recv_buff_.init(conf.base.recv_buff_max_size, SESSION_BUFFER_TYPE_RECV))
		return false;
	if (!send_buff_.init(conf.base.send_buff_max_size, SESSION_BUFFER_TYPE_SEND))
		return false;
	return true;
}

void JmyTcpConnector::connect(const ip::tcp::endpoint& ep)
{
	sock_->async_connect(ep, [this](boost::system::error_code err){
		if (err) {
			if (err.value() != boost::system::errc::operation_in_progress) {
				std::cerr << "JmyTcpConnector::connect  " << boost::system::system_error(err).what() << std::endl;
			}
			sock_->close();
			return;
		}
		sock_->set_option(ip::tcp::no_delay(conf_.is_delay));
	});
}

int JmyTcpConnector::send(const char* data, unsigned int len)
{
	return 0;
}

int JmyTcpConnector::run()
{
	return 0;
}

/**
 * JmyTcpMultiSameConnectors
 */
JmyTcpMultiSameConnectors::JmyTcpMultiSameConnectors() : curr_id_(0)
{
}

JmyTcpMultiSameConnectors::JmyTcpMultiSameConnectors(io_service& service, const ip::tcp::endpoint& ep) : ep_(ep), curr_id_(0)
{
}

JmyTcpMultiSameConnectors::~JmyTcpMultiSameConnectors()
{
}

bool JmyTcpMultiSameConnectors::init(io_service& service)
{
	return true;
}
