#include "jmy_tcp_connector.h"
#include <iostream>

JmyTcpConnector::JmyTcpConnector(io_service& service)
	: sock_(service), state_(CONNECTOR_STATE_NOT_CONNECT), sending_(false)
{
}

JmyTcpConnector::JmyTcpConnector(io_service& service, const ip::tcp::endpoint& ep)
	: sock_(service), ep_(ep), state_(CONNECTOR_STATE_NOT_CONNECT), sending_(false)
{
}

JmyTcpConnector::~JmyTcpConnector()
{
	destroy();
}

void JmyTcpConnector::close()
{
	sock_.close();
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
	if (state_ == CONNECTOR_STATE_CONNECTED) return;
	ip::tcp::endpoint ep(ip::address::from_string(ip), port);
	sock_.async_connect(ep, [this](boost::system::error_code err){
		if (err) {
			if (err.value() != boost::system::errc::operation_in_progress) {
				sock_.close();
				std::cerr << "JmyTcpConnector::connect  " << boost::system::system_error(err).what() << std::endl;
			}
			return;
		}
		sock_.set_option(ip::tcp::no_delay(conf_.no_delay));
		state_ = CONNECTOR_STATE_CONNECTED;
		if (conf_.connected_start) {
			start();
		}
	});
	state_ = CONNECTOR_STATE_CONNECTING;
}

void JmyTcpConnector::connect(const char* ip, short port)
{
	if (state_ == CONNECTOR_STATE_CONNECTED) return;
	const ip::tcp::endpoint ep(ip::address::from_string(ip), port);
	sock_.connect(ep);
	state_ = CONNECTOR_STATE_CONNECTED;
}

void JmyTcpConnector::start()
{
	if (state_ != CONNECTOR_STATE_CONNECTED) {
		std::cout << "JmyTcpConnector::start  socket not connected" << std::endl;
		return;
	}

	sock_.async_read_some(boost::asio::buffer(recv_buff_.getWriteBuff(), recv_buff_.getWriteLen()),
		[this](const boost::system::error_code& err, size_t bytes_transferred) {
			if (!err) {
				if (bytes_transferred > 0) {
					recv_buff_.writeLen(bytes_transferred);
				}
				if (bytes_transferred == 0) {
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
				}
				std::cout << "JmyTcpConnector::start  received " << bytes_transferred << " data" << std::endl;
				int nread = handler_.processData(recv_buff_, this);
				if (nread < 0) {
					std::cout << "JmyTcpConnector::start  handle_read failed" << std::endl;
					return;
				}
				recv_buff_.readLen(nread);
				start();
			} else {
				int ev = err.value();
				if (ev == 10053 || ev == 10054) {

				}
				sock_.close();
				std::cout << "JmyTcpConnector::start  read some data failed, err: " << err << std::endl;
			}
		} );
}

int JmyTcpConnector::send(int msg_id, const char* data, unsigned int len)
		{
	if (state_ != CONNECTOR_STATE_CONNECTED) return -1;
	int res = handler_.writeData(send_buff_, msg_id, data, len);
	if (res < 0) {
		std::cout << "JmyTcpConnector::send  write data length(" << len << ") failed" << std::endl;
		return -1;
	}
	return len;
}

int JmyTcpConnector::handle_send()
{
	if (state_ != CONNECTOR_STATE_CONNECTED) return -1;
	if (send_buff_.getReadLen() == 0)
		return 0;

	//if (sending_)
		//return 0;

	sock_.async_write_some(boost::asio::buffer(send_buff_.getReadBuff(), send_buff_.getReadLen()),
		[this](const boost::system::error_code& err, size_t bytes_transferred){
			if (!err) {
				if (!send_buff_.readLen(bytes_transferred)) {
					std::cout << "JmyTcpConnector::handle_send  send buff read len " << bytes_transferred << " failed" << std::endl;
					return;
				}
				std::cout << "JmyTcpConnector::handle_send  send " << bytes_transferred << " bytes" << std::endl;
			} else {
				sock_.close();
				state_ = CONNECTOR_STATE_DISCONNECT;
				std::cout << "JmyTcpConnector::handle_send  async_send error: " << err << std::endl;
				return;
			}
			//sending_ = false;
		});
	//sending_ = true;
	return 1;
}

int JmyTcpConnector::run()
{
	int res = 0;
	if (state_ == CONNECTOR_STATE_CONNECTED)
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
