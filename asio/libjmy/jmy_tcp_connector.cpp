#include "jmy_tcp_connector.h"
#include "jmy_log.h"

JmyTcpConnector::JmyTcpConnector(io_service& service)
	: sock_(service), state_(CONNECTOR_STATE_NOT_CONNECT), starting_(false), sending_(false)
{
}

JmyTcpConnector::JmyTcpConnector(io_service& service, const ip::tcp::endpoint& ep)
	: sock_(service), ep_(ep), state_(CONNECTOR_STATE_NOT_CONNECT), starting_(false), sending_(false)
{
}

JmyTcpConnector::~JmyTcpConnector()
{
	destroy();
}

void JmyTcpConnector::close()
{
	starting_ = false;
	sending_ = false;
	sock_.close();
	state_ = CONNECTOR_STATE_DISCONNECT;
}

void JmyTcpConnector::destroy()
{
	close();
	recv_buff_.destroy();
	send_buff_.destroy();
}

void JmyTcpConnector::reset()
{
}

bool JmyTcpConnector::loadConfig(const JmyConnectorConfig& conf)
{
	if (!handler_.loadMsgHandle(conf.handlers, conf.nhandlers)) {
		LibJmyLogError("failed to load msg handle");
		return false;
	}
	if (!recv_buff_.init(conf.base.recv_buff_max_size, SESSION_BUFFER_TYPE_RECV))
		return false;
	if (!send_buff_.init(conf.base.send_buff_max_size, SESSION_BUFFER_TYPE_SEND))
		return false;

	return true;
}

void JmyTcpConnector::asynConnect(const char* ip, short port)
{
	if (state_ == CONNECTOR_STATE_CONNECTED) return;
	const ip::tcp::endpoint ep(ip::address::from_string(ip), port);
	sock_.async_connect(ep, [this, ep](boost::system::error_code err) {
		if (err) {
			if (err.value() != boost::system::errc::operation_in_progress) {
				close();
				LibJmyLogError("connect failed: %s", boost::system::system_error(err).what());
			}
			return;
		}
		ep_ = ep;
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
	ep_ = ep;
	state_ = CONNECTOR_STATE_CONNECTED;
}

void JmyTcpConnector::start()
{
	if (state_ != CONNECTOR_STATE_CONNECTED) {
		LibJmyLogError("socket not connected");
		return;
	}

	sock_.async_read_some(boost::asio::buffer(recv_buff_.getWriteBuff(), recv_buff_.getWriteLen()),
		[this](const boost::system::error_code& err, size_t bytes_transferred) {
			if (!err) {
				if (bytes_transferred > 0) {
					recv_buff_.writeLen(bytes_transferred);
					// recv bytes add to net tool
					tool_.addDownStream(bytes_transferred);
				}
				if (bytes_transferred == 0) {
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
				}
				LibJmyLogDebug("received %d data", bytes_transferred);
				int nread = handler_.processData(recv_buff_, this);
				if (nread < 0) {
					LibJmyLogError("handle_read failed");
					return;
				}
				start();
			} else {
				int ev = err.value();
				if (ev == 10053 || ev == 10054) {

				}
				close();
				LibJmyLogError("read some data failed, err: %d", err.value());
			}
		} );

	// net tool start
	tool_.start();
	starting_ = true;
}

int JmyTcpConnector::send(int msg_id, const char* data, unsigned int len)
		{
	if (state_ != CONNECTOR_STATE_CONNECTED) return -1;
	int res = handler_.writeData(send_buff_, msg_id, data, len);
	if (res < 0) {
		LibJmyLogError("write data length(%d) failed", len);
		return -1;
	}
	return len;
}

int JmyTcpConnector::handle_send()
{
	if (state_ != CONNECTOR_STATE_CONNECTED) return -1;
	if (send_buff_.getReadLen() == 0)
		return 0;

	if (sending_)
		return 0;

	sock_.async_write_some(boost::asio::buffer(send_buff_.getReadBuff(), send_buff_.getReadLen()),
		[this](const boost::system::error_code& err, size_t bytes_transferred){
			if (!err) {
				if (bytes_transferred > 0) {
					if (!send_buff_.readLen(bytes_transferred)) {
						LibJmyLogError("send buff read len %d failed", bytes_transferred);
						return;
					}
					tool_.addUpStream(bytes_transferred);
				}
				LibJmyLogDebug("send %d bytes", bytes_transferred);
			} else {
				close();
				LibJmyLogError("async_send error: ", err);
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
	if (state_ == CONNECTOR_STATE_CONNECTED)
		res = handle_send();
	static std::chrono::system_clock::time_point last_tick = std::chrono::system_clock::now();
	if (starting_) {
		auto now = std::chrono::system_clock::now();
		if (std::chrono::duration_cast<std::chrono::milliseconds>(now - last_tick).count() >= 1000) {
			last_tick = now;
			LibJmyLogInfo("send_Bps: %d, recv_Bps: %d, Bps: %d", tool_.getBps(), tool_.getRecvBps(), tool_.getSendBps());
		}
	}

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
