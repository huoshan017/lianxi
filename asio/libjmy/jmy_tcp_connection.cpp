#include "jmy_tcp_connection.h"
#include "jmy_mem.h"
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
	if (state_ != JMY_CONN_STATE_CONNECTED)
		return;
	state_ = JMY_CONN_STATE_DISCONNECTING;
	active_close_start_ = std::chrono::system_clock::now();
	if (sendDisconnect() < 0) return;
	sending_data_ = false;
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

			handle_recv();

			start();
		} else {
			int ev = err.value();
			if (ev == 10053 || ev == 10054) {

			}
			close();
			state_ = JMY_CONN_STATE_DISCONNECTED;
			LibJmyLogError("read some data failed, err: %d", err.value());
		}
	});

	if (conn_type_ == JMY_CONN_TYPE_PASSIVE && state_ != JMY_CONN_STATE_CONNECTED) {
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

int JmyTcpConnection::sendDisconnect()
{
	if (!buffer_->use_send_list) {
		if (handler_->writeDisconnect(&buffer_->send_buff) < 0) {
			LibJmyLogError("write disconnect failed");
			return -1;
		}
	} else {
		if (handler_->writeDisconnect(&buffer_->send_buff_list) < 0) {
			LibJmyLogError("write disconnect failed");
			return -1;
		}
	}
	return 0;
}

int JmyTcpConnection::sendDisconnectAck()
{
	if (!buffer_->use_send_list) {
		if (handler_->writeDisconnectAck(&buffer_->send_buff) < 0) {
			LibJmyLogError("write disconnect ack failed");
			return -1;
		}
	} else {
		if (handler_->writeDisconnectAck(&buffer_->send_buff_list) < 0) {
			LibJmyLogError("write disconnect ack failed");
			return -1;
		}
	}
	return 0;
}

int JmyTcpConnection::handleAck()
{
	return 0;
}

int JmyTcpConnection::handleHeartbeat()
{
	return 0;
}

int JmyTcpConnection::handleDisconnect()
{
	sendDisconnectAck();
	return 0;
}

int JmyTcpConnection::handleDisconnectAck()
{
	if (state_ != JMY_CONN_STATE_DISCONNECTING)
		return 0;

	sock_.close();
	return 1;
}

int JmyTcpConnection::handle_recv()
{
	// data process
	int count = handler_->processData(buffer_->recv_buff, id_, unused_data_);
	if (count < 0) {
		LibJmyLogError("handle_recv failed");
		return -1;
	}
	return count;
}

int JmyTcpConnection::handle_send()
{
	if (state_ != JMY_CONN_STATE_CONNECTED)
		return -1;

	unsigned int read_len = 0;
	read_len = buffer_->use_send_list ? buffer_->send_buff_list.getReadLen() : buffer_->send_buff.getReadLen();

	if (read_len == 0) {
		return 0;
	}

	if (sending_data_)
		return 0;

	if (!buffer_->use_send_list) {
		sock_.async_write_some(
				boost::asio::buffer(buffer_->send_buff.getReadBuff(), buffer_->send_buff.getReadLen()),
				[this](const boost::system::error_code& err, size_t bytes_transferred) {
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
		sock_.async_write_some(
				boost::asio::buffer(buffer_->send_buff_list.getReadBuff(), buffer_->send_buff_list.getReadLen()),
				[this](const boost::system::error_code& err, size_t bytes_transferred) {
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

int JmyTcpConnection::run()
{
	if (state_ == JMY_CONN_STATE_DISCONNECTING) {
		std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
		if (std::chrono::duration_cast<std::chrono::seconds>(now-active_close_start_).count() >= JMY_ACTIVE_CLOSE_CONNECTION_TIMEOUT) {
			sock_.close();
			state_ = JMY_CONN_STATE_DISCONNECTED;
		}
	}
	if (state_ != JMY_CONN_STATE_CONNECTED)
		return -1;

	return handle_send();
}

/**
 * JmyTcpConnectionMgr
 */
JmyTcpConnectionMgr::JmyTcpConnectionMgr()
{
}

JmyTcpConnectionMgr::~JmyTcpConnectionMgr()
{
	clear();
}

void JmyTcpConnectionMgr::clear()
{
	JmyTcpConnection* conn = nullptr;
	std::list<JmyTcpConnection*>::iterator it = free_list_.begin();
	for (; it!=free_list_.end(); ++it) {
		conn = *it;
		if (conn) {
			jmy_mem_free(conn);
		}
	}
	free_list_.clear();
	std::unordered_map<int, JmyTcpConnection*>::iterator uit = used_map_.begin();
	for (; uit!=used_map_.end(); ++uit) {
		conn = *it;
		if (conn) {
			jmy_mem_free(conn);
		}
	}
	used_map_.clear();
}

void JmyTcpConnectionMgr::init(int size)
{
	int i = 0;
	for (; i<size; ++i) {
		free_list_.push_back((JmyTcpConnection*)jmy_mem_malloc<JmyTcpConnection>());
	}
	size_ = size;
}

JmyTcpConnection* JmyTcpConnectionMgr::getFree(int id)
{
	std::unordered_map<int, JmyTcpConnection*>::iterator it = used_map_.find(id);
	if (it != used_map_.end()) {
		LibJmyLogWarn("id(%d) is used", id);
		return nullptr;
	}

	if (free_list_.size() == 0) {
		LibJmyLogWarn("no free connection to get");
		return nullptr;
	}

	JmyTcpConnection* conn = free_list_.front();
	conn->setId(id);
	free_list_.pop_front();
	used_map_.insert(std::make_pair(id, conn));
	return conn;
}

JmyTcpConnection* JmyTcpConnectionMgr::get(int id)
{
	std::unordered_map<int, JmyTcpConnection*>::iterator it = used_map_.find(id);
	if (it == used_map_.end()) return nullptr;
	return it->second;
}

bool JmyTcpConnectionMgr::free(JmyTcpConnection* conn)
{
	if (!conn) return false;
	std::unordered_map<int, JmyTcpConnection*>::iterator it = used_map_.find(conn->getId());
	if (it == used_map_.end()) return false;
	used_map_.erase(conn->getId());
	free_list_.push_back(conn);
	return true;
}

int JmyTcpConnectionMgr::usedRun()
{
	for (auto c: used_map_) {
		if (!c.second) continue;
		if (c.second->isDisconnect()) {
			free(c.second);
		}
		c.second->run();
	}
	return 0;
}
