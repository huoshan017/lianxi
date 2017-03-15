#include "jmy_tcp_connector.h"
#include "jmy_log.h"
#include <thread>

JmyTcpConnector::JmyTcpConnector(io_service& service, JmyTcpConnectorMgr& mgr) :
	mgr_(mgr),
	sock_(service),
	state_(CONNECTOR_STATE_NOT_CONNECT),
	use_send_list_(false),
	starting_(false),
	sending_(false),
	unused_data_(NULL)
{
}

JmyTcpConnector::JmyTcpConnector(io_service& service, JmyTcpConnectorMgr& mgr, const ip::tcp::endpoint& ep) :
	mgr_(mgr),
	sock_(service),
	ep_(ep),
	state_(CONNECTOR_STATE_NOT_CONNECT),
	use_send_list_(false),
	starting_(false),
	sending_(false),
	unused_data_(NULL)
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
	if (!handler_.loadMsgHandle(conf.common.handlers, conf.common.nhandlers)) {
		LibJmyLogError("failed to load msg handle");
		return false;
	}
	if (!recv_buff_.init(conf.common.recv_buff_max_size, SESSION_BUFFER_TYPE_RECV))
		return false;
	if (!send_buff_.init(conf.common.send_buff_max_size, SESSION_BUFFER_TYPE_SEND))
		return false;

	use_send_list_ = conf.common.use_send_list;
	if (use_send_list_) {
		send_buff_list_.init(0, 0);
	}
	LibJmyLogInfo("use send list enable(%d)", (int)use_send_list_);

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
		sock_.set_option(ip::tcp::no_delay(conf_.common.no_delay));
		state_ = CONNECTOR_STATE_CONNECTED;
		if (conf_.common.connected_start) {
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

int JmyTcpConnector::check_reconn_info_for_recv(unsigned short recv_count)
{
	JmyReconnectConfig& conf = conf_.common.reconn_conf;
	// ack count add recv count beyond the id scope
	if (reconn_info_.ack_recv_msg_count + recv_count > conf.range.max-conf.range.min+1) {
		LibJmyLogError("received msg count %d is great to max id %d",
				reconn_info_.ack_recv_msg_count + recv_count, conf.range.max-conf.range.min);
		return -1;
	}

	unsigned short curr_id = reconn_info_.curr_ack_recv_id;
	// curr recv id rotate
	if (curr_id + recv_count > conf.range.max)
		curr_id = curr_id + recv_count - conf.range.max + conf.range.min;

	// receive count reached ack count limit
	if (reconn_info_.ack_recv_msg_count + recv_count >= conf.ack_recv_count) {
		if (!use_send_list_) {
			int res = handler_.writeAck(&send_buff_, recv_count, curr_id);
			if (res < 0) {
				LibJmyLogError("write ack (recv_count:%d, curr_id:%d) failed", recv_count, curr_id);
				return -1;
			}
		} else {
			int res = handler_.writeAck(&send_buff_list_, recv_count, curr_id);
			if (res < 0) {
				LibJmyLogError("write ack (recv_count:%d, curr_id:%d) failed", recv_count, curr_id);
				return -1;
			}
		}
		reconn_info_.ack_recv_msg_count = 0;
		reconn_info_.curr_ack_recv_id = curr_id;
	} else {
		reconn_info_.ack_recv_msg_count += recv_count;
	}
	return 0;
}

void JmyTcpConnector::start()
{
	if (state_ != CONNECTOR_STATE_CONNECTED) {
		LibJmyLogError("socket not connected");
		return;
	}

	sock_.async_read_some(boost::asio::buffer(recv_buff_.getWriteBuff(), recv_buff_.getWriteLen()), [this](const boost::system::error_code& err, size_t bytes_transferred) {
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

			{
				// data process
				int count = handler_.processData(recv_buff_, getId(), (void*)&mgr_);
				if (count < 0) {
					LibJmyLogError("handle_read failed");
					return;
				}
				// check receive info to ack
				if (check_reconn_info_for_recv(count) < 0) {
					return;
				}
			}

			start();
		} else {
			int ev = err.value();
			if (ev == 10053 || ev == 10054) {

			}
			close();
			state_ = CONNECTOR_STATE_DISCONNECT;
			LibJmyLogError("read some data failed, err: %d", err.value());
		}
	});

	// net tool start
	tool_.start();
	if (!starting_) {
		starting_ = true;
		last_tick_ = std::chrono::system_clock::now();
	}
}

int JmyTcpConnector::send(int msg_id, const char* data, unsigned int len)
		{
	if (state_ != CONNECTOR_STATE_CONNECTED) return -1;
	if (!use_send_list_) {
		int res = handler_.writeData(send_buff_, msg_id, data, len);
		if (res < 0) {
			LibJmyLogError("write data length(%d) failed", len);
			return -1;
		}
	} else {
		int res = handler_.writeData(&send_buff_list_, msg_id, data, len);
		if (res < 0) {
			LibJmyLogError("write data length(%d) failed", len);
			return -1;
		}
	}
	return len;
}

int JmyTcpConnector::handle_send()
{
	if (state_ != CONNECTOR_STATE_CONNECTED) return -1;

	unsigned int read_len = 0;
	read_len = use_send_list_ ? send_buff_list_.getReadLen() : send_buff_.getReadLen();

	if (read_len == 0) {
		LibJmyLogDebug("read len is 0");
		return 0;
	}

	if (sending_)
		return 0;

	if (!use_send_list_) {
		sock_.async_write_some(boost::asio::buffer(send_buff_.getReadBuff(), send_buff_.getReadLen()), [this](const boost::system::error_code& err, size_t bytes_transferred) {
			if (!err) {
				if (bytes_transferred > 0) {
					if (!send_buff_.readLen(bytes_transferred)) {
						LibJmyLogError("send buff read len %d failed", bytes_transferred);
						return;
					}
					tool_.addUpStream(bytes_transferred);
				}
				LibJmyLogInfo("connector(%d) send %d bytes", getId(), bytes_transferred);
			} else {
				close();
				state_ = CONNECTOR_STATE_DISCONNECT;
				LibJmyLogError("connector(%d) async_write_some error: %d", getId(), err.value());
				return;
			}
			sending_ = false;
		});
	} else {
		sock_.async_write_some(boost::asio::buffer(send_buff_list_.getReadBuff(), send_buff_list_.getReadLen()), [this](const boost::system::error_code& err, size_t bytes_transferred) {
			if (!err) {
				if (bytes_transferred > 0) {
					if (!send_buff_list_.readLen(bytes_transferred)) {
						LibJmyLogError("send buff list read lne %d failed", bytes_transferred);
						return;
					}
					tool_.addUpStream(bytes_transferred);
				}
				LibJmyLogDebug("connector(%d) send %d bytes", getId(), bytes_transferred);
			} else {
				close();
				state_ = CONNECTOR_STATE_DISCONNECT;
				LibJmyLogError("connector(%d) async_send error: %d", getId(), err.value());
				return;
			}	
			sending_ = false;
		});
	}
	sending_ = true;
	return 1;
}

int JmyTcpConnector::run()
{
	int res = 0;
	if (state_ == CONNECTOR_STATE_CONNECTED)
		res = handle_send();

	if (starting_) {
		auto now = std::chrono::system_clock::now();
		if (std::chrono::duration_cast<std::chrono::milliseconds>(now - last_tick_).count() >= 1000) {
			last_tick_ = now;
			LibJmyLogDebug("id(%d)  send_Bps: %d, recv_Bps: %d, Bps: %d", getId(), tool_.getRecvBps(), tool_.getSendBps(), tool_.getBps());
		}
	}

	return res;
}

/**
 * JmyTcpConnectorMgr
 */
JmyTcpConnectorMgr::JmyTcpConnectorMgr() : use_auto_id_(false), use_multi_threads_(false), curr_id_(0)
{
}

JmyTcpConnectorMgr::~JmyTcpConnectorMgr()
{
}

void JmyTcpConnectorMgr::clear()
{
	std::unordered_map<int, JmyTcpConnector*>::iterator it = conns_.begin();
	for (; it!=conns_.end(); ++it) {
		if (it->second) {
			delete it->second;
		}
	}
	conns_.clear();
}

void JmyTcpConnectorMgr::init(bool use_multi_threads, bool use_auto_id)
{
	use_multi_threads_ = use_multi_threads;
	use_auto_id_ = use_auto_id;
}

JmyTcpConnector* JmyTcpConnectorMgr::newConnector(io_service& service)
{
	if (!use_auto_id_)
		return 0;

	if (use_multi_threads_) {
		std::lock_guard<std::mutex> lock(mtx_);
	}
	JmyTcpConnector* conn = new JmyTcpConnector(service, *this);
	curr_id_ += 1;
	conns_.insert(std::make_pair(curr_id_, conn));
	conn->conf_.conn_id = curr_id_;
	LibJmyLogInfo("new connector(%d)", curr_id_);
	return conn;
}

JmyTcpConnector* JmyTcpConnectorMgr::newConnector(io_service& service, int id)
{
	if (use_auto_id_)
		return NULL;

	if (use_multi_threads_) {
		std::lock_guard<std::mutex> lock(mtx_);
	}
	if (conns_.find(id) != conns_.end())
		return NULL;
	JmyTcpConnector* conn = new JmyTcpConnector(service, *this);
	conns_.insert(std::make_pair(id, conn));
	return conn;
}

JmyTcpConnector* JmyTcpConnectorMgr::get(int id)
{
	if (use_multi_threads_) {
		std::lock_guard<std::mutex> lock(mtx_);
	}
	std::unordered_map<int, JmyTcpConnector*>::iterator it = conns_.find(id);
	if (it == conns_.end())
		return NULL;

	if (!it->second) {
		conns_.erase(id);
	}
	return it->second;
}

bool JmyTcpConnectorMgr::check(int id, bool locked)
{
	if (!locked && use_multi_threads_) {
		std::lock_guard<std::mutex> lock(mtx_);
	}
	std::unordered_map<int, JmyTcpConnector*>::iterator it = conns_.find(id);
	if (it == conns_.end())
		return false;
	if (!it->second) {
		conns_.erase(id);
		return false;
	}
	return true;
}

bool JmyTcpConnectorMgr::remove(int id)
{
	if (use_multi_threads_) {
		std::lock_guard<std::mutex> lock(mtx_);
	}
	if (!check(id, true)) {
		LibJmyLogWarn("not found connector(%d), remove failed", id);
		return false;
	}
	conns_.erase(id);
	return true;
}

bool JmyTcpConnectorMgr::remove(JmyTcpConnector* conn)
{
	if (use_multi_threads_) {
		std::lock_guard<std::mutex> lock(mtx_);
	}

	bool found = false;
	std::unordered_map<int, JmyTcpConnector*>::iterator it = conns_.begin();
	for (; it!=conns_.end(); ++it) {
		if (it->second == conn) {
			found = true;
			break;
		}
	}
	conns_.erase(it);
	return true;
}

/**
 * JmyTcpMultiConnectors
 */
JmyTcpMultiConnectors::JmyTcpMultiConnectors(io_service& service, int max_count)
	: service_(service), max_count_(max_count)
{
}

JmyTcpMultiConnectors::JmyTcpMultiConnectors(io_service& service, const ip::tcp::endpoint& ep, int max_count)
	: service_(service), max_count_(max_count), ep_(ep)
{
}

JmyTcpMultiConnectors::~JmyTcpMultiConnectors()
{
	destroy();
}

void JmyTcpMultiConnectors::close()
{
	used_ids_.clear();
	free_ids_.clear();
}

void JmyTcpMultiConnectors::destroy()
{
	close();
	max_count_ = 0;
}

void JmyTcpMultiConnectors::reset()
{
}

bool JmyTcpMultiConnectors::loadConfig(const JmyMultiConnectorsConfig& conf)
{
	if (conf.max_count <= 0) {
		LibJmyLogError("conf max_count %d is invalid", conf.max_count);
		return false;
	}
	conf_ = conf;
	mgr_.init(true, true);
	return true;
}

JmyTcpConnector* JmyTcpMultiConnectors::start(const char* ip, short port)
{
	size_t s = used_ids_.size();
	if ((int)s >= max_count_) {
		LibJmyLogWarn("current connector count %d is max", (int)s);
		return 0;
	}

	JmyTcpConnector* conn = NULL;
	int id = 0;
	if (free_ids_.size() == 0) {
		conn = mgr_.newConnector(service_);
		if (!conn) {
			LibJmyLogError("insert connector failed");
			return NULL;
		}
		id = conn->getId();
		LibJmyLogInfo("connector id = %d", id);
	} else {
		id = free_ids_.front();
		conn = mgr_.get(id);
		if (!conn) {
			LibJmyLogError("get connector(%d) failed", id);
			return NULL;
		}
		conn->reset();
	}

	JmyConnectorConfig conf;
	conf.conn_id = id;
	conf.assign(conf_);
	if (!conn->loadConfig(conf)) {
		LibJmyLogError("connector load config failed");
		return 0;
	}
	// blocking connect
	conn->connect(ip, port);
	conn->start();
	used_ids_.insert(id);
	return conn;
}

JmyTcpConnector* JmyTcpMultiConnectors::getConnector(int connector_id)
{
	std::set<int>::iterator it = used_ids_.find(connector_id);
	if (it == used_ids_.end())
		return NULL;

	return mgr_.get(connector_id);
}

JmyConnectorState JmyTcpMultiConnectors::getState(int connector_id)
{
	JmyTcpConnector* conn = getConnector(connector_id);
	if (!conn) {
		return CONNECTOR_STATE_NOT_CONNECT;
	}
	return conn->getState();
}

int JmyTcpMultiConnectors::send(int connector_id, int msg_id, const char* data, unsigned int len)
{
	JmyTcpConnector* conn = getConnector(connector_id);
	if (!conn)
		return 0;

	int res = conn->send(msg_id, data, len);
	if (res < 0) {
		used_ids_.erase(connector_id);
		free_ids_.push_back(connector_id);
		conn->close();
		conn->reset();
	}
	return res;
}

int JmyTcpMultiConnectors::run(int connector_id)
{
	JmyTcpConnector* conn = getConnector(connector_id);
	if (!conn)
		return -1;

	int res = conn->run();
	if (res < 0) {
		used_ids_.erase(connector_id);
		free_ids_.push_back(connector_id);
	}
	return res;
}

int JmyTcpMultiConnectors::startInturn(int count, const char* ip, short port)
{
	int c = 0;
	count = max_count_ < count ? max_count_: count;
	for (; c<count; ++c) {
		if (start(ip, port) == NULL) {
			break;
		}
	}
	return c;
}

struct connector_send {
	connector_send(JmyTcpConnectorMgr& mgr, int msg_id, const char* data, unsigned int len)
		: mgr_(mgr),  msg_id_(msg_id), data_(data), len_(len), count_(0)
	{
	}
	void operator()(int id) {
		JmyTcpConnector* conn = mgr_.get(id);
		if (!conn) return;
		if (conn->send(msg_id_, data_, len_) > 0) {
			count_ += 1;	
		}
	}
	int get_count() const { return count_; }
private:
	JmyTcpConnectorMgr& mgr_;
	int msg_id_;
	const char* data_;
	unsigned int len_;
	int count_;
};

int JmyTcpMultiConnectors::sendInturn(int msg_id, const char* data, unsigned int len)
{
	connector_send cs(mgr_, msg_id, data, len);
	std::for_each(used_ids_.begin(), used_ids_.end(), cs);
	return cs.get_count();
}

int JmyTcpMultiConnectors::runInturn()
{
	int c = 0;
	std::set<int>::iterator it = used_ids_.begin();
	for (; it!=used_ids_.end(); ++it) {
		JmyTcpConnector* conn = getConnector(*it);
		if (conn) {
			if (conn->run() >= 0)
				c += 1;
		}
	}
	return c;
}
