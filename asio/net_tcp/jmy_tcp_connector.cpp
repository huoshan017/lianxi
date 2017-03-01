#include "jmy_tcp_connector.h"
#include <iostream>

JmyTcpConnector::JmyTcpConnector() : state_(CONNECTOR_STATE_NOT_CONNECT), sending_(false)
{
}

JmyTcpConnector::JmyTcpConnector(io_service& service, const ip::tcp::endpoint& ep)
	: ep_(ep), state_(CONNECTOR_STATE_NOT_CONNECT), sending_(false)
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
	state_ = CONNECTOR_STATE_DISCONNECT;
}

void JmyTcpConnector::destroy()
{
	close();
}

void JmyTcpConnector::reset()
{
}

bool JmyTcpConnector::loadConfig(const JmyConnectorConfig& conf)
{
	if (!handler_.loadMsgHandle(conf.handlers, conf.nhandlers)) {
		std::cout << "JmyTcpConnector::loadConfig  failed to load msg handle" << std::endl;
		return false;
	}
	if (!recv_buff_.init(conf.base.recv_buff_max_size, SESSION_BUFFER_TYPE_RECV))
		return false;
	if (!send_buff_.init(conf.base.send_buff_max_size, SESSION_BUFFER_TYPE_SEND))
		return false;
	return true;
}

void JmyTcpConnector::async_connect(const char* ip, short port)
{
	ip::tcp::endpoint ep(ip::address::from_string(ip), port);
	sock_->async_connect(ep, [this](boost::system::error_code err){
		if (err) {
			if (err.value() != boost::system::errc::operation_in_progress) {
				sock_->close();
				std::cerr << "JmyTcpConnector::connect  " << boost::system::system_error(err).what() << std::endl;
			}
			return;
		}
		sock_->set_option(ip::tcp::no_delay(conf_.is_delay));
		state_ = CONNECTOR_STATE_CONNETED;
	});
	state_ = CONNECTOR_STATE_CONNECTING;
}

void JmyTcpConnector::connect(const char* ip, short port)
{
	const ip::tcp::endpoint ep(ip::address::from_string(ip), port);
	sock_->connect(ep);
	state_ = CONNECTOR_STATE_CONNETED;
}

void JmyTcpConnector::start()
{
	if (state_ != CONNECTOR_STATE_CONNETED) {
		std::cout << "JmyTcpConnector::start  socket not connected" << std::endl;
		return;
	}

	sock_->async_read_some(boost::asio::buffer(recv_buff_.getWriteBuff(), recv_buff_.getWriteLen()),
		[this](const boost::system::error_code& err, size_t bytes_transferred) {
			std::cout << "222222" << std::endl;
			if (!err) {
				if (bytes_transferred > 0) {
					recv_buff_.writeLen(bytes_transferred);
				}
				std::cout << "JmyTcpConnector::start  received " << bytes_transferred << " data" << std::endl;
				int nread = handler_.processData(recv_buff_, shared_from_this().get());
				if (nread < 0) {
					std::cout << "JmyTcpSession::start handle_read failed" << std::endl;
					return;
				}
				start();
			} else {
				int ev = err.value();
				if (ev == 10053 || ev == 10054) {

				}
				std::cout << "JmyTcpSession::start  read some data failed, err: " << err << std::endl;
			}
		} );
}

int JmyTcpConnector::send(int msg_id, const char* data, unsigned int len)
{
	int res = handler_.writeData(send_buff_, msg_id, data, len);
	if (res < 0) {
		std::cout << "JmyTcpSession::send write data length(" << len << ") failed" << std::endl;
		return -1;
	}
	return len;
}

int JmyTcpConnector::handle_send()
{
	if (send_buff_.getReadLen() == 0)
		return 0;

	if (sending_)
		return 0;

	sock_->async_write_some(boost::asio::buffer(send_buff_.getReadBuff(), send_buff_.getReadLen()),
			[this](const boost::system::error_code& err, size_t bytes_transferred){
				if (!err) {
					send_buff_.readLen(bytes_transferred);
				} else {
					std::cout << "JmyTcpSession::handle_write  async_send error: " << err << std::endl;
					return;
				}
				sending_ = false;
			});
	sending_ = true;
	return 1;
}

int JmyTcpConnector::run()
{
	int res = 0;
	if (state_ == CONNECTOR_STATE_CONNETED)
		res = handle_send();
	return res;
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
