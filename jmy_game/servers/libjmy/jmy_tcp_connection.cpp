#include "jmy_tcp_connection.h"
#include "jmy_mem.h"
#include "jmy_log.h"
#include <thread>
#if USE_COROUTINE
#include <boost/asio/spawn.hpp>
#endif

JmyTcpConnection::JmyTcpConnection(io_service& service, JmyTcpConnectionMgr& mgr, JmyConnType conn_type)
	: id_(0), strand_(service), 
#if USE_COROUTINE
	coroutine_running_(false),
#endif
	sock_(service), mgr_(mgr), conn_type_(conn_type), state_(JMY_CONN_STATE_NOT_USE), sending_data_(false), buff_send_count_(0)
{
}

JmyTcpConnection::~JmyTcpConnection()
{
	destroy();
}

void JmyTcpConnection::close()
{
	if (state_ != JMY_CONN_STATE_CONNECTED)
		return;

	if (sendDisconnect() < 0) return;
	active_close_start_ = std::chrono::system_clock::now();
	state_ = JMY_CONN_STATE_DISCONNECTING;
}

void JmyTcpConnection::force_close()
{
	if (state_ == JMY_CONN_STATE_DISCONNECTED)
		return;

	sock_.close();
	if (buffer_.get())
		buffer_->clear();
	state_ = JMY_CONN_STATE_DISCONNECTED;
	sending_data_ = false;
#if USE_COROUTINE
	coroutine_running_ = false;
#endif
	handle_event(JMY_EVENT_DISCONNECT, 0);
}

void JmyTcpConnection::destroy()
{
}

void JmyTcpConnection::reset()
{
	state_ = JMY_CONN_STATE_NOT_CONNECT;
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
				state_ = JMY_CONN_STATE_NOT_CONNECT;
				LibJmyLogError("connect failed: %s", boost::system::system_error(err).what());
			}
			return;
		}
		ep_ = ep;
		sock_.set_option(ip::tcp::no_delay(false));
		state_ = JMY_CONN_STATE_CONNECTED;
		start();
		handle_event(JMY_EVENT_CONNECT, 0);
	});
	state_ = JMY_CONN_STATE_CONNECTING;
}

bool JmyTcpConnection::connect(const char* ip, short port)
{
	if (conn_type_ != JMY_CONN_TYPE_ACTIVE) {
		LibJmyLogWarn("connection type is not active");
		return false;
	}
	if (state_ == JMY_CONN_STATE_CONNECTED) {
		LibJmyLogInfo("connection already connected");
		return false;
	}
	const ip::tcp::endpoint ep(ip::address::from_string(ip), port);
	try {
		sock_.connect(ep);
	}
	catch (std::exception& e) {
		LibJmyLogError("connect %s:%d failed, error(%s)", ip, port, e.what());
		return false;
	}
	ep_ = ep;
	state_ = JMY_CONN_STATE_CONNECTED;
	handle_event(JMY_EVENT_CONNECT, 0);
	return true;
}

void JmyTcpConnection::start()
{
	start_recv();
	//start_send();
}

void JmyTcpConnection::start_recv()
{
	if (conn_type_ == JMY_CONN_TYPE_ACTIVE && state_ != JMY_CONN_STATE_CONNECTED) {
		LibJmyLogWarn("active connection's state is not connected");
		return;
	}

	if (!data_handler_.get() || !buffer_.get()) {
		LibJmyLogError("handler(0x%x) or buffer(0x%x) not set", data_handler_.get(), buffer_.get());
		return;
	}

	sock_.async_read_some(
			boost::asio::buffer(buffer_->recv_buff.getWriteBuff(), buffer_->recv_buff.getWriteLen()),
			[this](const boost::system::error_code& err, size_t bytes_transferred) {
		if (!err) {
			buffer_->recv_buff.writeLen(bytes_transferred);
#if 0
			if (handle_recv() < 0) {
				force_close();
				return;
			}
#endif
			start_recv();
		} else {
			int ev = err.value();
			if (ev == boost::system::errc::no_such_file_or_directory) {
				LibJmyLogInfo("peer(%s:%d) is closed", sock_.remote_endpoint().address().to_string().c_str(), sock_.remote_endpoint().port());
			} else if (ev == boost::system::errc::operation_canceled) {
				LibJmyLogInfo("operation was canceled");
				return;
			} else {
				LibJmyLogError("read some data failed, err_code(%d), err_str(%s)", ev, err.message().c_str());
			}
			force_close();
		}
	});

	if (conn_type_ == JMY_CONN_TYPE_PASSIVE && state_ != JMY_CONN_STATE_CONNECTED) {
		state_ = JMY_CONN_STATE_CONNECTED;
	}
}

void JmyTcpConnection::start_list_recv()
{
	if (conn_type_ == JMY_CONN_TYPE_ACTIVE && state_ != JMY_CONN_STATE_CONNECTED) {
		LibJmyLogWarn("active connection's state is not connected");
		return;
	}

	if (!data_handler_.get() || !buffer_.get()) {
		LibJmyLogError("handler(0x%x) or buffer(0x%x) not set", data_handler_.get(), buffer_.get());
		return;
	}

	char packet_type;
	sock_.async_receive(
			boost::asio::buffer(&packet_type, 1),
			[this, packet_type](const boost::system::error_code& err, size_t bytes_transferred) {
		if (!err) {
			if (bytes_transferred != 1) {
				force_close();
				LibJmyLogError("receive packet type failed");
				return;
			}
			handle_packet((JmyPacketType)packet_type);
		} else {
			int ev = err.value();
			if (ev == boost::system::errc::no_such_file_or_directory) {
				LibJmyLogInfo("peer(%s:%d) is closed", sock_.remote_endpoint().address().to_string().c_str(), sock_.remote_endpoint().port());
			} else if (ev == boost::system::errc::operation_canceled) {
				LibJmyLogInfo("operation was canceled");
				return;
			} else {
				LibJmyLogError("read some data failed, err_code(%d), err_str(%s)", ev, err.message().c_str());
			}
			force_close();
		}
	});

	if (conn_type_ == JMY_CONN_TYPE_PASSIVE && state_ != JMY_CONN_STATE_CONNECTED) {
		state_ = JMY_CONN_STATE_CONNECTED;
	}
}

void JmyTcpConnection::handle_packet(JmyPacketType packet_type)
{
	if (packet_type == JMY_PACKET_DISCONNECT) {
		handleDisconnect();
	} else if (packet_type == JMY_PACKET_DISCONNECT_ACK) {
		handleDisconnectAck();
	} else if (packet_type == JMY_PACKET_HEARTBEAT) {
		handleHeartbeat();
	} else if (packet_type == JMY_PACKET_USER_DATA) {
	} else if (packet_type == JMY_PACKET_USER_ID_DATA) {
	}
}

int JmyTcpConnection::send(int msg_id, const char* data, unsigned int len)
{
	if (state_ != JMY_CONN_STATE_CONNECTED)
		return -1;

	if (!buffer_->use_send_list) {
		int res = data_handler_->writeUserData(&buffer_->send_buff, msg_id, data, len);
		if (res < 0) {
			LibJmyLogError("write data length(%d) failed", len);
			return -1;
		}
	} else {
		int res = data_handler_->writeUserData(&buffer_->send_buff_list, msg_id, data, len);
		if (res < 0) {
			LibJmyLogError("write data length(%d) failed", len);
			return -1;
		}
	}
	return len;	
}

int JmyTcpConnection::send(int user_id, int msg_id, const char* data, unsigned short len)
{
	if (state_ != JMY_CONN_STATE_CONNECTED)
		return -1;

	if (!buffer_->use_send_list) {
		int res = data_handler_->writeUserIdAndData(&buffer_->send_buff, user_id, msg_id, data, len);
		if (res < 0) {
			LibJmyLogError("write data length(%d) failed", len);
			return -1;
		}
	} else {
		int res = data_handler_->writeUserIdAndData(&buffer_->send_buff_list, user_id, msg_id, data, len);
		if (res < 0) {
			LibJmyLogError("write data length(%d) failed", len);
			return -1;
		}
	}
	return len;
}

#if 0
int JmyTcpConnection::sendAck(JmyAckInfo* info)
{
	if (state_ != JMY_CONN_STATE_CONNECTED)
		return -1;

	if (!buffer_->use_send_list) {
		if (data_handler_->writeAck(&buffer_->send_buff, info->ack_count, info->curr_id) < 0) {
			LibJmyLogError("write ack failed");
			return -1;
		}
	} else {
		if (data_handler_->writeAck(&buffer_->send_buff_list, info->ack_count, info->curr_id) < 0) {
			LibJmyLogError("write ack failed");
			return -1;
		}
	}
	return 0;
}
#endif

int JmyTcpConnection::sendHeartbeat()
{
	if (state_ != JMY_CONN_STATE_CONNECTED)
		return -1;

	if (!buffer_->use_send_list) {
		if (data_handler_->writeHeartbeat(&buffer_->send_buff) < 0) {
			LibJmyLogError("write heart beat failed");
			return -1;
		}
	} else {
		if (data_handler_->writeHeartbeat(&buffer_->send_buff_list) < 0) {
			LibJmyLogError("write heart beat failed");
			return -1;
		}
	}
	return 0;
}

int JmyTcpConnection::sendDisconnect()
{
	if (!buffer_->use_send_list) {
		if (data_handler_->writeDisconnect(&buffer_->send_buff) < 0) {
			LibJmyLogError("write disconnect failed");
			return -1;
		}
	} else {
		if (data_handler_->writeDisconnect(&buffer_->send_buff_list) < 0) {
			LibJmyLogError("write disconnect failed");
			return -1;
		}
	}
	return 0;
}

int JmyTcpConnection::sendDisconnectAck()
{
	if (!buffer_->use_send_list) {
		if (data_handler_->writeDisconnectAck(&buffer_->send_buff) < 0) {
			LibJmyLogError("write disconnect ack failed");
			return -1;
		}
	} else {
		if (data_handler_->writeDisconnectAck(&buffer_->send_buff_list) < 0) {
			LibJmyLogError("write disconnect ack failed");
			return -1;
		}
	}
	return 0;
}

#if 0
int JmyTcpConnection::handleAck(JmyAckInfo* info)
{
	if (info->ack_count > 0) {
		if (buffer_->use_send_list) {
			if (buffer_->send_buff_list.getUsedSize() < info->ack_count) {
				LibJmyLogError("send_buff_list used size(%d) not enough to ack acount(%d)",
						buffer_->send_buff_list.getUsedSize(), info->ack_count);
				return -1;
			}
			buffer_->send_buff_list.dropUsed(info->ack_count);
			buffer_->total_reconn_info.send_count -= info->ack_count;
		} else {
			// todo
			LibJmyLogInfo("to do later");
		}
	}
	return 0;
}
#endif

int JmyTcpConnection::handleHeartbeat()
{
	return 0;
}

int JmyTcpConnection::handleDisconnect()
{
	if (state_ == JMY_CONN_STATE_DISCONNECTING) {
		force_close();
		return 0;
	}
	return sendDisconnectAck();
}

int JmyTcpConnection::handleDisconnectAck()
{
	if (state_ != JMY_CONN_STATE_DISCONNECTING) {
		LibJmyLogWarn("state(%d) not JMY_CONN_STATE_DISCONNECTING(%d)", state_, JMY_CONN_STATE_DISCONNECTING);
		return 0;
	}

	force_close();
	return 1;
}

int JmyTcpConnection::handle_recv()
{
	// data process
	int count = data_handler_->processData(buffer_->recv_buff, id_, &mgr_);
	if (count < 0) {
		LibJmyLogError("handle_recv failed");
		return -1;
	}

	return count;
}

void JmyTcpConnection::buffer_send()
{
	sock_.async_send(
			boost::asio::buffer(buffer_->send_buff.getReadBuff(), buffer_->send_buff.getReadLen()),
			[this](const boost::system::error_code& err, size_t bytes_transferred) {
		if (!err) {
			if (bytes_transferred > 0) {
				if (!buffer_->send_buff.readLen(bytes_transferred)) {
					LibJmyLogError("send buff read len %d failed", bytes_transferred);
					return;
				}
			}
		} else {
			force_close();
			LibJmyLogError("connection(%d) async_write_some error: %d", id_, err.value());
			return;
		}
		sending_data_ = false;
	});
}

void JmyTcpConnection::buffer_list_send()
{
	sock_.async_send(
			boost::asio::buffer(buffer_->send_buff_list.getReadBuff(), buffer_->send_buff_list.getReadLen()),
			[this](const boost::system::error_code& err, size_t bytes_transferred) {
		if (err) {
			force_close();
			LibJmyLogError("connection(%d) async_write_some error: %d(%s)", id_, err.value(), err.message().c_str());
			return;
		}

		buffer_->send_buff_list.readLen(bytes_transferred);
		buff_send_count_ += 1;
		if (buff_send_count_ >= 100 || !buffer_->send_buff_list.getUsingSize()) {
			buff_send_count_ = 0;
			sending_data_ = false;
			LibJmyLogInfo("!!!!!!!!! end send buff list");
			return;
		}
		buffer_list_send();
		LibJmyLogInfo("%d bytes transferred, set sending false", bytes_transferred);
	});
}

int JmyTcpConnection::start_send()
{
	if (state_ != JMY_CONN_STATE_CONNECTED)
		return -1;

	if (sending_data_) {
		LibJmyLogInfo("sending data");
		return 0;
	}

	unsigned int read_len = buffer_->use_send_list ? buffer_->send_buff_list.getReadLen() : buffer_->send_buff.getReadLen();
	if (read_len == 0) {
		return 0;
	}
	if (!buffer_->use_send_list) {
		buffer_send();
	} else {
		buffer_list_send();
	}
	sending_data_ = true;
	return 1;
}

int JmyTcpConnection::handle_event(int event_id, long param)
{
	event_info_.event_id = event_id;
	event_info_.conn_id = id_;
	event_info_.param = (void*)&mgr_;
	event_info_.param_l = param;
	int res = 0;
	if (event_id == JMY_EVENT_CONNECT)
		res = event_handler_->onConnect(&event_info_);
	else if (event_id == JMY_EVENT_DISCONNECT)
		res = event_handler_->onDisconnect(&event_info_);
	else if (event_id == JMY_EVENT_TICK)
		res = event_handler_->onTick(&event_info_);
	else if (event_id == JMY_EVENT_TIMER)
		res = event_handler_->onTimer(&event_info_);
	else
		res = event_handler_->onEvent(&event_info_);
	return res;
}

int JmyTcpConnection::run()
{
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	if (event_handler_->hasTickHandler()) {
		long elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - last_run_tick_).count();
		handle_event(JMY_EVENT_TICK, elapsed);
		last_run_tick_ = now;
	}

	if (state_ == JMY_CONN_STATE_DISCONNECTING) {
		if (std::chrono::duration_cast<std::chrono::seconds>(now-active_close_start_).count() >= JMY_ACTIVE_CLOSE_CONNECTION_TIMEOUT) {
			force_close();
			return 0;
		}
	}

	if (state_ != JMY_CONN_STATE_CONNECTED)
		return 0;

#if USE_COROUTINE
	int res = 0;
	if (!coroutine_running_) {
		if (go()) {
			coroutine_running_ = true;
			res = 1;
		}
	}
	return res;
#else
	if (buffer_->recv_buff.getReadLen() > 0) {
		if (handle_recv() < 0) {
			force_close();
			return -1;
		}
	}
	return start_send();
#endif
}

#if USE_COROUTINE
JmyTcpConnection::recv_coro::recv_coro(JmyTcpConnection* conn) : conn_(conn)
{
}

JmyTcpConnection::recv_coro::~recv_coro()
{
}

#if 0
void JmyTcpConnection::recv_coro::operator()(boost::system::error_code ec, std::size_t bytes_transferred)
{
	if (!conn_->data_handler_.get() || !conn_->buffer_.get()) {
		LibJmyLogError("handler(0x%x) or buffer(0x%x) not set", conn_->data_handler_.get(), conn_->buffer_.get());
		return;
	}

	if (ec) {
		int ev = ec.value();
		if (ev == boost::system::errc::no_such_file_or_directory) {
			LibJmyLogInfo("peer(%s:%d) is closed", conn_->sock_.remote_endpoint().address().to_string().c_str(), conn_->sock_.remote_endpoint().port());
		} else if (ev == boost::system::errc::operation_canceled) {
			LibJmyLogInfo("operation was canceled");
			return;
		} else {
			LibJmyLogError("read some data failed, err_code(%d), err_str(%s)", ev, ec.message().c_str());
		}
		conn_->force_close();
		return;
	}

	BOOST_ASIO_CORO_REENTER (this)
	{
		while (true) {
			BOOST_ASIO_CORO_YIELD conn_->sock_.async_read_some(boost::asio::buffer(conn_->buffer_->recv_buff.getWriteBuff(), conn_->buffer_->recv_buff.getWriteLen()), *this);
			conn_->buffer_->recv_buff.writeLen(conn_->buffer_->recv_buff.getWriteLen());
			if (conn_->handle_recv() < 0) {
				conn_->force_close();
				return;
			}
			if (conn_->conn_type_ == JMY_CONN_TYPE_PASSIVE && conn_->state_ != JMY_CONN_STATE_CONNECTED) {
				conn_->state_ = JMY_CONN_STATE_CONNECTED;
			}	
		}
	}
}
#endif

JmyTcpConnection::send_coro::send_coro(JmyTcpConnection* conn) : conn_(conn)
{
}

JmyTcpConnection::send_coro::~send_coro()
{
}

void JmyTcpConnection::send_coro::operator()(boost::system::error_code ec, std::size_t bytes_transferred)
{
	if (ec) {
		conn_->force_close();
		LibJmyLogError("connection(%d) async_write_some error(%d), error_string(%s)", conn_->id_, ec.value(), ec.message().c_str());
		return;
	}

	BOOST_ASIO_CORO_REENTER (this)
	{
		while (true) {
			unsigned int read_len = conn_->buffer_->use_send_list ? conn_->buffer_->send_buff_list.getReadLen() : conn_->buffer_->send_buff.getReadLen();
			boost::system::error_code ec;
			if (conn_->buffer_->use_send_list) {
				BOOST_ASIO_CORO_YIELD conn_->sock_.async_send(boost::asio::buffer(conn_->buffer_->send_buff_list.getReadBuff(), read_len), *this);
			} else {
				BOOST_ASIO_CORO_YIELD conn_->sock_.async_send(boost::asio::buffer(conn_->buffer_->send_buff.getReadBuff(), read_len), *this);
			}

			if (conn_->buffer_->use_send_list) {
				if (!conn_->buffer_->send_buff_list.readLen(read_len)) {
					conn_->force_close();
					LibJmyLogError("send buff list read len %d failed", read_len);
					return;
				}
			} else {
				if (!conn_->buffer_->send_buff.readLen(read_len)) {
					conn_->force_close();
					LibJmyLogError("send buff read len %d failed", read_len);
					return;
				}
			}
		}
	}
}

int JmyTcpConnection::go()
{
	if (conn_type_ == JMY_CONN_TYPE_ACTIVE && state_ != JMY_CONN_STATE_CONNECTED) {
		LibJmyLogWarn("active connection's state is not connected");
		return -1;
	}

#if 0
	// read coroutine
	boost::asio::spawn(strand_,
		[this](boost::asio::yield_context yield) {
			if (!data_handler_.get() || !buffer_.get()) {
				LibJmyLogError("handler(0x%x) or buffer(0x%x) not set", data_handler_.get(), buffer_.get());
				return;
			}

			boost::system::error_code ec;
			while (true) {
				sock_.async_receive(boost::asio::buffer(buffer_->recv_buff.getWriteBuff(), buffer_->recv_buff.getWriteLen()), yield[ec]);
				if (ec) {
					int ev = ec.value();
					if (ev == boost::system::errc::no_such_file_or_directory) {
						LibJmyLogInfo("peer(%s:%d) is closed", sock_.remote_endpoint().address().to_string().c_str(), sock_.remote_endpoint().port());
					} else if (ev == boost::system::errc::operation_canceled) {
						LibJmyLogInfo("operation was canceled");
						return;
					} else {
						LibJmyLogError("read some data failed, err_code(%d), err_str(%s)", ev, ec.message().c_str());
					}
					force_close();
					return;
				}

				buffer_->recv_buff.writeLen(buffer_->recv_buff.getWriteLen());
				if (handle_recv() < 0) {
					force_close();
					return;
				}
				if (conn_type_ == JMY_CONN_TYPE_PASSIVE && state_ != JMY_CONN_STATE_CONNECTED) {
					state_ = JMY_CONN_STATE_CONNECTED;
				}	
			}
		}
	);

	// write coroutine
	boost::asio::spawn(strand_,
		[this](boost::asio::yield_context yield) {
			while (true) {
				unsigned int read_len = buffer_->use_send_list ? buffer_->send_buff_list.getReadLen() : buffer_->send_buff.getReadLen();
				boost::system::error_code ec;
				if (buffer_->use_send_list) {
					sock_.async_send(boost::asio::buffer(buffer_->send_buff_list.getReadBuff(), read_len), yield[ec]);
				} else {
					sock_.async_send(boost::asio::buffer(buffer_->send_buff.getReadBuff(), read_len), yield[ec]);
				}
				if (!ec) {
					if (buffer_->use_send_list) {
						if (!buffer_->send_buff_list.readLen(read_len)) {
							force_close();
							LibJmyLogError("send buff list read len %d failed", read_len);
							return;
						}
					} else {
						if (!buffer_->send_buff.readLen(read_len)) {
							force_close();
							LibJmyLogError("send buff read len %d failed", read_len);
							return;
						}
					}
				} else {
					force_close();
					LibJmyLogError("connection(%d) async_write_some error(%d), error_string(%s)", id_, ec.value(), ec.message().c_str());
					return;
				}
			}
		}
	);
#endif
	return 1;
}
#endif

/**
 * JmyTcpConnectionMgr
 */
JmyTcpConnectionMgr::JmyTcpConnectionMgr(io_service& service)
	: service_(service)
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
		conn = uit->second;
		if (conn) {
			jmy_mem_free(conn);
		}
	}
	used_map_.clear();
}

void JmyTcpConnectionMgr::init(int size, JmyConnType conn_type)
{
	int i = 0;
	for (; i<size; ++i) {
		JmyTcpConnection* conn = jmy_mem_malloc<JmyTcpConnection>(service_, *this, conn_type);
		free_list_.push_back(conn);
	}
	size_ = size;
}

void JmyTcpConnectionMgr::init(int size, JmyConnType conn_type, const JmyConnectionConfig& conf)
{
	init(size, conn_type);
	conf_ = conf;
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
	if (it == used_map_.end())
		return nullptr;
	return it->second;
}

bool JmyTcpConnectionMgr::free(JmyTcpConnection* conn)
{
	if (!conn)
		return false;

	std::unordered_map<int, JmyTcpConnection*>::iterator it = used_map_.find(conn->getId());
	if (it == used_map_.end())
		return false;

	used_map_.erase(conn->getId());
	free_list_.push_front(conn);
	conn->reset();
	LibJmyLogInfo("free connection(%d)", conn->getId());
	return true;
}
