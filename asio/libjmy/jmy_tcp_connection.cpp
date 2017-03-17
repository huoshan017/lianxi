#include "jmy_tcp_connection.h"
#include "jmy_log.h"
#include <thread>

JmyTcpConnection::JmyTcpConnection(io_service& service, JmyConnType conn_type)
	: id_(0), sock_(service), conn_type_(conn_type), state_(JMY_CONN_STATE_NOT_USE), sending_data_(false), unused_data_(nullptr)
{
	std::memset(&total_reconn_info_, 0, sizeof(total_reconn_info_));
}

JmyTcpConnection::~JmyTcpConnection()
{
	destroy();
}

void JmyTcpConnection::close()
{
	sending_data_ = false;
	sock_.close();
}

void JmyTcpConnection::destroy()
{
}

void JmyTcpConnection::reset()
{
}

void JmyTcpConnection::asynConnect(const char* ip, short port)
{
	if (conn_type_ != JMY_CONN_TYPE_ACTIVE) {
		LibJmyLogWarn("connection type is not active");
		return;
	}
	if (state_ == JMY_CONN_STATE_CONNECTED) {
		LibJmyLogInfo("connection already connected");
		return;
	}
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
		sock_.set_option(ip::tcp::no_delay(false));
		state_ = JMY_CONN_STATE_CONNECTED;
		start();
	});
	state_ = JMY_CONN_STATE_CONNECTING;
}

void JmyTcpConnection::connect(const char* ip, short port)
{
	if (conn_type_ != JMY_CONN_TYPE_ACTIVE) {
		LibJmyLogWarn("connection type is not active");
		return;
	}
	if (state_ == JMY_CONN_STATE_CONNECTED) {
		LibJmyLogInfo("connection already connected");
		return;
	}
	const ip::tcp::endpoint ep(ip::address::from_string(ip), port);
	sock_.connect(ep);
	ep_ = ep;
	state_ = JMY_CONN_STATE_CONNECTED;
}

void JmyTcpConnection::start()
{
	if (conn_type_ == JMY_CONN_TYPE_ACTIVE && state_ != JMY_CONN_STATE_CONNECTED) {
		LibJmyLogWarn("active connection's state is not connected");
		return;
	}

	if (!handler_.get() || !buffer_.get()) {
		LibJmyLogError("handler or buffer not set");
		return;
	}

	sock_.async_read_some(
			boost::asio::buffer(buffer_->recv_buff.getWriteBuff(), buffer_->recv_buff.getWriteLen()),
			[this](const boost::system::error_code& err, size_t bytes_transferred) {
		if (!err) {
			if (bytes_transferred > 0) {
				buffer_->recv_buff.writeLen(bytes_transferred);
			}
			if (bytes_transferred == 0) {
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}
			LibJmyLogDebug("received %d data", bytes_transferred);

			{
				// data process
				int count = handler_->processData(buffer_->recv_buff, id_, unused_data_);
				if (count < 0) {
					LibJmyLogError("handle_read failed");
					return;
				}
			}

			start();
		} else {
			int ev = err.value();
			if (ev == 10053 || ev == 10054) {

			}
			close();
			state_ = JMY_CONN_STATE_DISCONNECTING;
			LibJmyLogError("read some data failed, err: %d", err.value());
		}
	});

	if (conn_type_ == JMY_CONN_TYPE_PASSIVE) {
		state_ = JMY_CONN_STATE_CONNECTED;
	}
}

int JmyTcpConnection::send(int msg_id, const char* data, unsigned int len)
{
	if (state_ != JMY_CONN_STATE_CONNECTED)
		return -1;

	if (!buffer_->use_send_list) {
		int res = handler_->writeData(&buffer_->send_buff, msg_id, data, len);
		if (res < 0) {
			LibJmyLogError("write data length(%d) failed", len);
			return -1;
		}
	} else {
		int res = handler_->writeData(&buffer_->send_buff_list, msg_id, data, len);
		if (res < 0) {
			LibJmyLogError("write data length(%d) failed", len);
			return -1;
		}
	}
	return len;	
}

int JmyTcpConnection::run()
{
	return handle_send();
}

int JmyTcpConnection::sendAck(JmyAckInfo* info)
{
	if (state_ != JMY_CONN_STATE_CONNECTED)
		return -1;

	if (!buffer_->use_send_list) {
		if (handler_->writeAck(&buffer_->send_buff, info->ack_count, info->curr_id) < 0) {
			LibJmyLogError("write ack failed");
			return -1;
		}
	} else {
		if (handler_->writeAck(&buffer_->send_buff_list, info->ack_count, info->curr_id) < 0) {
			LibJmyLogError("write ack failed");
			return -1;
		}
	}
	return 0;
}

int JmyTcpConnection::sendHeartbeat()
{
	if (state_ != JMY_CONN_STATE_CONNECTED)
		return -1;

	if (!buffer_->use_send_list) {
		if (handler_->writeHeartbeat(&buffer_->send_buff) < 0) {
			LibJmyLogError("write heart beat failed");
			return -1;
		}
	} else {
		if (handler_->writeHeartbeat(&buffer_->send_buff_list) < 0) {
			LibJmyLogError("write heart beat failed");
			return -1;
		}
	}
	return 0;
}

int JmyTcpConnection::handle_recv()
{
	return 0;
}

int JmyTcpConnection::handle_send()
{
	if (state_ != JMY_CONN_STATE_CONNECTED)
		return -1;

	unsigned int read_len = 0;
	read_len = buffer_->use_send_list ? buffer_->send_buff_list.getReadLen() : buffer_->send_buff.getReadLen();

	if (read_len == 0) {
		LibJmyLogDebug("read len is 0");
		return 0;
	}

	if (sending_data_)
		return 0;

	if (!buffer_->use_send_list) {
		sock_.async_write_some(boost::asio::buffer(buffer_->send_buff.getReadBuff(), buffer_->send_buff.getReadLen()), [this](const boost::system::error_code& err, size_t bytes_transferred) {
			if (!err) {
				if (bytes_transferred > 0) {
					if (!buffer_->send_buff.readLen(bytes_transferred)) {
						LibJmyLogError("send buff read len %d failed", bytes_transferred);
						return;
					}
				}
				LibJmyLogInfo("connector(%d) send %d bytes", getId(), bytes_transferred);
			} else {
				close();
				state_ = JMY_CONN_STATE_DISCONNECTED;
				LibJmyLogError("connector(%d) async_write_some error: %d", id_, err.value());
				return;
			}
			sending_data_ = false;
		});
	} else {
		sock_.async_write_some(boost::asio::buffer(buffer_->send_buff_list.getReadBuff(), buffer_->send_buff_list.getReadLen()), [this](const boost::system::error_code& err, size_t bytes_transferred) {
			if (!err) {
				if (bytes_transferred > 0) {
					if (!buffer_->send_buff_list.readLen(bytes_transferred)) {
						LibJmyLogError("send buff list read lne %d failed", bytes_transferred);
						return;
					}
				}
				LibJmyLogDebug("connector(%d) send %d bytes", getId(), bytes_transferred);
			} else {
				close();
				state_ = JMY_CONN_STATE_DISCONNECTED;
				LibJmyLogError("connector(%d) async_write_some error: %d", id_, err.value());
				return;
			}	
			sending_data_ = false;
		});
	}
	sending_data_ = true;
	return 1;
}
