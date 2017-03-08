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
	if (!recv_buff_.init(conf.recv_buff_max_size, SESSION_BUFFER_TYPE_RECV))
		return false;
	if (!send_buff_.init(conf.send_buff_max_size, SESSION_BUFFER_TYPE_SEND))
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
				state_ = CONNECTOR_STATE_DISCONNECT;
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
				state_ = CONNECTOR_STATE_DISCONNECT;
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
 * JmyTcpMultiConnectors
 */
JmyTcpMultiConnectors::JmyTcpMultiConnectors(io_service& service, int max_count)
	: service_(service), max_count_(max_count), curr_id_(0)
{
}

JmyTcpMultiConnectors::JmyTcpMultiConnectors(io_service& service, const ip::tcp::endpoint& ep, int max_count)
	: service_(service), max_count_(max_count), curr_id_(0), ep_(ep)
{
}

JmyTcpMultiConnectors::~JmyTcpMultiConnectors()
{
	destroy();
}

void JmyTcpMultiConnectors::close()
{
	auto it = id2conn_.begin();
	for (; it!=id2conn_.end(); ++it) {
		if (it->second) {
			it->second->close();
		}
	}
}

void JmyTcpMultiConnectors::destroy()
{
	std::unordered_map<int, JmyTcpConnector*>::iterator it = id2conn_.begin();
	for (; it!=id2conn_.end(); ++it) {
		if (it->second) {
			it->second->destroy();
		}
	}
	id2conn_.clear();
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
	return true;
}

int JmyTcpMultiConnectors::start(const char* ip, short port)
{
	size_t s = id2conn_.size();
	if ((int)s >= max_count_) {
		LibJmyLogWarn("current connector count %d is max", (int)s);
		return 0;
	}

	JmyTcpConnector* conn = NULL;
	if (free_conn_.size() == 0) {
		conn = new JmyTcpConnector(service_);
	} else {
		conn = free_conn_.front().second;		
		conn->reset();
	}
	if (!conn->loadConfig(conf_.config)) {
		LibJmyLogError("connector load config failed");
		return 0;
	}
	// blocking connect
	conn->connect(ip, port);
	conn->start();
	curr_id_ += 1;
	if (curr_id_ > MaxId) curr_id_ = 1;
	id2conn_.insert(std::make_pair(curr_id_, conn));
	return curr_id_;
}

JmyTcpConnector* JmyTcpMultiConnectors::getConnector(int connector_id)
{
	std::unordered_map<int, JmyTcpConnector*>::iterator it = id2conn_.find(connector_id);
	if (it == id2conn_.end())
		return NULL;

	return it->second;
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
		id2conn_.erase(connector_id);
	}
	return res;
}

int JmyTcpMultiConnectors::run(int connector_id)
{
	JmyTcpConnector* conn = getConnector(connector_id);
	if (!conn)
		return -1;

	return conn->run();
}

int JmyTcpMultiConnectors::startInturn(int count, const char* ip, short port)
{
	int c = 0;
	count = max_count_ < count ? max_count_: count;
	for (; c<count; ++c) {
		if (start(ip, port) < 0) {
			break;
		}
	}
	return c;
}

struct connector_send {
	connector_send(int msg_id, const char* data, unsigned int len)
		: msg_id_(msg_id), data_(data), len_(len), count_(0)
	{
	}
	template <class T>
	void operator()(T& t) {
		if (t.second->send(msg_id_, data_, len_) > 0) {
			count_ += 1;	
		}
	}
	int get_count() const { return count_; }
private:
	int msg_id_;
	const char* data_;
	unsigned int len_;
	int count_;
};

int JmyTcpMultiConnectors::sendInturn(int msg_id, const char* data, unsigned int len)
{
	connector_send cs(msg_id, data, len);
	std::for_each(id2conn_.begin(), id2conn_.end(), cs);
	return cs.get_count();
}

int JmyTcpMultiConnectors::runInturn()
{
	int c = 0;
	std::unordered_map<int, JmyTcpConnector*>::iterator it = id2conn_.begin();
	for (; it!=id2conn_.end(); ++it) {
		if (it->second) {
			if (it->second->run() > 0)
				c += 1;
		}
	}
	return c;
}
