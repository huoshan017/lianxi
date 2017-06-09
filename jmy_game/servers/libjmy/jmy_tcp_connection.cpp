#include "jmy_tcp_connection.h"
#include "jmy_mem.h"
#include "jmy_log.h"
#include <thread>

JmyTcpConnection::JmyTcpConnection(io_service& service, JmyTcpConnectionMgr& mgr, JmyConnType conn_type)
	: id_(0), sock_(service), mgr_(mgr), conn_type_(conn_type), state_(JMY_CONN_STATE_NOT_USE), sending_data_(false)//, unused_data_(nullptr)
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
	sending_data_ = false;
}

void JmyTcpConnection::force_close()
{
	if (state_ == JMY_CONN_STATE_DISCONNECTED)
		return;

	sock_.close();
	if (buffer_.get())
		buffer_->clear();
	state_ = JMY_CONN_STATE_DISCONNECTED;
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
			if (bytes_transferred > 0) {
				buffer_->recv_buff.writeLen(bytes_transferred);
				if (handle_recv() < 0) {
					force_close();
					return;
				}	
			}
			else if (bytes_transferred == 0) {
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}
			//LibJmyLogDebug("received %d data", bytes_transferred);
			start();
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

#if 0
	// ack recv data count
	JmyResendConfig* rc = mgr_.getConf().retran_conf;
	if (rc) {
		if (buffer_->total_reconn_info.recv_count + count >= rc->ack_recv_count) {
			JmyAckInfo ack_info;
			ack_info.ack_count = buffer_->total_reconn_info.recv_count + count;
			if (this->sendAck(&ack_info) < 0) {
				LibJmyLogError("send ack failed");
				return -1;
			}
			buffer_->total_reconn_info.recv_count = 0;
		} else {
			buffer_->total_reconn_info.recv_count += count;
		}
	}
#endif
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
				//LibJmyLogInfo("connection(%d) send %d bytes", getId(), bytes_transferred);
			} else {
				force_close();
				LibJmyLogError("connection(%d) async_write_some error: %d", id_, err.value());
				return;
			}
			sending_data_ = false;
		});
	} else {
		sock_.async_write_some(
				boost::asio::buffer(buffer_->send_buff_list.getReadBuff(), buffer_->send_buff_list.getReadLen()),
				[this](const boost::system::error_code& err, size_t bytes_transferred) {
			if (err) {
				force_close();
				LibJmyLogError("connection(%d) async_write_some error: %d", id_, err.value());
				return;
			}
			
			sending_data_ = false;
			if (bytes_transferred > 0) {
				JmyPacketType pt = (JmyPacketType)buffer_->send_buff_list.readLen(bytes_transferred);
				if (pt > 0) {
					// check send data count
					/*JmyResendConfig* rc = mgr_.getConf().retran_conf;
					if (rc && buffer_->total_reconn_info.send_count+1 >= rc->max_cached_send_count) {
						force_close();
						LibJmyLogError("cached send buffer count is max");
						return;
					}

					if (pt == JMY_PACKET_USER_DATA) {
						buffer_->total_reconn_info.send_count += 1;
						//LibJmyLogInfo("send list count %d", total_reconn_info_.send_count);
					}
					*/
				}
				// continue send
				handle_send();
			}
		});
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
		return -1;

	return handle_send();
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
	LibJmyLogInfo("free connection(%d)", conn->getId());
	return true;
}

int JmyTcpConnectionMgr::usedRun()
{
	int n = 0;
	std::unordered_map<int, JmyTcpConnection*>::iterator it, tmp_it;
	it = used_map_.begin();
	for (; it!=used_map_.end(); ) {
		tmp_it = it;
		tmp_it++;
		bool del = false;
		if (!it->second || it->second->isDisconnect()) {
			del = true;
		}
		if (del && it->second) {
			free(it->second);
			it = tmp_it;
			continue;
		}
		if (it->second->run() >= 0)
			n += 1;
		it = tmp_it;
	}
	return n;
}
