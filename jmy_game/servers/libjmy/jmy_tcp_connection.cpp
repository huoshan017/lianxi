#include "jmy_tcp_connection.h"
#include "jmy_mem.h"
#include "jmy_log.h"
#include <thread>

JmyTcpConnection::JmyTcpConnection(io_service& service, JmyTcpConnectionMgr& mgr, JmyConnType conn_type)
	: id_(0), strand_(service), 
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

int JmyTcpConnection::handle_recv_error(const boost::system::error_code& err)
{
	int ev = err.value();
	if (ev == boost::system::errc::no_such_file_or_directory) {
		if (sock_.available()) {
			LibJmyLogInfo("peer(%s:%d) is closed", sock_.remote_endpoint().address().to_string().c_str(), sock_.remote_endpoint().port());
		}
	} else if (ev == boost::system::errc::operation_canceled) {
		LibJmyLogInfo("operation was canceled");
	} else {
		LibJmyLogError("read some data failed, err_code(%d), err_str(%s)", ev, err.message().c_str());
	}
	force_close();
	return 0;
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

#if USE_NET_PROTO2
	boost::asio::async_read(sock_,
			boost::asio::buffer(tmp_, sizeof(tmp_)), boost::asio::transfer_all(),
			[this](const boost::system::error_code& err, size_t bytes_transferred) {
		if (!err) {
			if (bytes_transferred != sizeof(tmp_)) {
				force_close();
				LibJmyLogError("bytes transferred %d failed", bytes_transferred);
				return;
			}
			JmyPacketType2 type = JMY_PACKET2_NONE;
			int len = 0;
			if (!jmy_net_proto2_get_packet_len_type(tmp_, sizeof(tmp_), type, len)) {
				force_close();
				LibJmyLogError("get packet type and len failed");
				return;
			}
			if (handle_packet_type(type) == 0) {
				start_recv();
				return;
			}
			recv_packet(type, len-JMY_PACKET2_LEN_TYPE);
#else
	sock_.async_read_some(
			boost::asio::buffer(buffer_->recv_buff.getWriteBuff(), buffer_->recv_buff.getWriteLen()),
			[this](const boost::system::error_code& err, size_t bytes_transferred) {
		if (!err) {
			buffer_->recv_buff.writeLen(bytes_transferred);
			if (handle_recv() < 0) {
				force_close();
				return;
			}
			start_recv();
#endif
		} else {
			handle_recv_error(err);
		}
	});

	if (conn_type_ == JMY_CONN_TYPE_PASSIVE && state_ != JMY_CONN_STATE_CONNECTED) {
		state_ = JMY_CONN_STATE_CONNECTED;
	}
}

#if USE_NET_PROTO2
void JmyTcpConnection::recv_packet(JmyPacketType2 type, unsigned short pack_len)
{
	char* buf = (char*)jmy_mem_malloc(pack_len);
	boost::asio::async_read(sock_,
		boost::asio::buffer(buf, pack_len), boost::asio::transfer_all(),
		[this, type, buf, pack_len](const boost::system::error_code& err, size_t bytes_transferred) {
		if (!err) {
			if (bytes_transferred != pack_len) {
				force_close();
				LibJmyLogError("receive packet len(%d) not equal to (%d) failed", bytes_transferred, pack_len);
				return;
			}

			buffer_->recv_buff_list.writeData((unsigned short)type, buf, pack_len);
			start_recv();
		} else {
			jmy_mem_free(buf);
			handle_recv_error(err);
		}
	});
}
#endif

int JmyTcpConnection::handle_packet_type(JmyPacketType2 packet_type)
{
	if (packet_type == JMY_PACKET2_DISCONNECT) {
		handleDisconnect();
	} else if (packet_type == JMY_PACKET2_DISCONNECT_ACK) {
		handleDisconnectAck();
	} else if (packet_type == JMY_PACKET2_HEARTBEAT) {
		handleHeartbeat();
	} else if (packet_type == JMY_PACKET2_USER_DATA) {
		return 1;
	} else if (packet_type == JMY_PACKET2_USER_ID_DATA) {
		return 1;
	}
	return 0;
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
#if USE_NET_PROTO2
	int count = data_handler_->processData(&buffer_->recv_buff_list, id_, &mgr_);
#else
	int count = data_handler_->processData(buffer_->recv_buff, id_, &mgr_);
#endif
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
			return;
		}
		buffer_list_send();
	});
}

int JmyTcpConnection::start_send()
{
	if (state_ != JMY_CONN_STATE_CONNECTED)
		return -1;

	if (sending_data_) {
		LibJmyLogDebug("sending data");
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

	if (handle_recv() < 0) {
		force_close();
		return -1;
	}
	return start_send();
}

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
	LibJmyLogDebug("free connection(%d)", conn->getId());
	return true;
}
