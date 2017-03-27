#include "jmy_tcp_clients.h"

JmyTcpClients::JmyTcpClients(io_service& service, int max_count)
	: max_count_(max_count), mgr_(service)
{
	handler_ = std::make_shared<JmyDataHandler>();
	buff_pool_ = std::make_shared<JmySessionBufferPool>();
}

JmyTcpClients::JmyTcpClients(io_service& service, const ip::tcp::endpoint& ep, int max_count)
	: max_count_(max_count), ep_(ep), mgr_(service)
{
	handler_ = std::make_shared<JmyDataHandler>();
	buff_pool_ = std::make_shared<JmySessionBufferPool>();
}

JmyTcpClients::~JmyTcpClients()
{
	destroy();
}

void JmyTcpClients::close()
{
	used_ids_.clear();
	free_ids_.clear();
}

void JmyTcpClients::destroy()
{
	close();
	max_count_ = 0;
}

void JmyTcpClients::reset()
{
}

bool JmyTcpClients::loadConfig(const JmyClientsConfig& conf)
{
	if (conf.max_conn <= 0) {
		LibJmyLogError("conf max_count %d is invalid", conf.max_conn);
		return false;
	}

	mgr_.init(conf.max_conn, JMY_CONN_TYPE_ACTIVE, conf.conn_conf);

	if (!handler_->loadMsgHandle(conf.conn_conf.handlers, conf.conn_conf.nhandlers)) {
		LibJmyLogError("load msg handler failed");		
		return false;
	}

	if (!buff_pool_->init(2*conf.max_conn,
			conf.conn_conf.buff_conf.send_buff_size,
			conf.conn_conf.buff_conf.send_buff_size,
			conf.conn_conf.buff_conf.recv_buff_size,
			conf.conn_conf.buff_conf.recv_buff_size)) {
		LibJmyLogError("init buff pool failed");		
		return false;
	}

	buffer_mgr_.init(2*conf.max_conn, buff_pool_);
	conf_ = conf;
	return true;
}

JmyTcpConnection* JmyTcpClients::start()
{
	size_t s = used_ids_.size();
	if ((int)s >= max_count_) {
		LibJmyLogWarn("current connection count %d is max", (int)s);
		return 0;
	}

	std::shared_ptr<JmyConnectionBuffer> buffer;
	JmyTcpConnection* conn = NULL;
	int id = 0;
	if (free_ids_.size() == 0) {
		id = id_gen_.get();
		conn = mgr_.getFree(id);
		if (!conn) {
			LibJmyLogError("insert connection failed");
			return nullptr;
		}
		LibJmyLogDebug("get free conn");
		id = conn->getId();
		if (!buffer_mgr_.getOneBuffer(buffer)) {
			LibJmyLogError("get free buffer failed");
			return nullptr;
		}
		buffer->init(conf_.conn_conf.buff_conf, buff_pool_);
		conn->setBuffer(buffer);
		conn->setDataHandler(handler_);
		LibJmyLogInfo("connection id = %d", id);
	} else {
		id = free_ids_.front();
		conn = mgr_.get(id);
		if (!conn) {
			LibJmyLogError("get connection(%d) failed", id);
			return NULL;
		}
		conn->reset();
	}

	// blocking connect
	if (!conn->connect(conf_.conn_ip, conf_.conn_port)) {
		return nullptr;
	}
	LibJmyLogDebug("connect success");
	conn->start();
	used_ids_.insert(id);
	return conn;
}

JmyTcpConnection* JmyTcpClients::getConnection(int conn_id)
{
	std::set<int>::iterator it = used_ids_.find(conn_id);
	if (it == used_ids_.end())
		return NULL;

	return mgr_.get(conn_id);
}

JmyConnState JmyTcpClients::getState(int conn_id)
{
	JmyTcpConnection* conn = getConnection(conn_id);
	if (!conn) {
		return JMY_CONN_STATE_NOT_CONNECT;
	}
	return conn->getConnState();
}

int JmyTcpClients::send(int conn_id, int msg_id, const char* data, unsigned int len)
{
	JmyTcpConnection* conn = getConnection(conn_id);
	if (!conn)
		return 0;

	int res = conn->send(msg_id, data, len);
	if (res < 0) {
		used_ids_.erase(conn_id);
		free_ids_.push_back(conn_id);
		conn->close();
		conn->reset();
	}
	return res;
}

int JmyTcpClients::run(int conn_id)
{
	JmyTcpConnection* conn = getConnection(conn_id);
	if (!conn)
		return -1;

	int res = conn->run();
	if (res < 0) {
		used_ids_.erase(conn_id);
		free_ids_.push_back(conn_id);
	}
	return res;
}

int JmyTcpClients::startInturn(int count)
{
	int c = 0;
	count = max_count_ < count ? max_count_: count;
	for (; c<count; ++c) {
		if (start() == NULL) {
			break;
		}
	}
	return c;
}

struct connection_send {
	connection_send(JmyTcpConnectionMgr& mgr, int msg_id, const char* data, unsigned int len)
		: mgr_(mgr),  msg_id_(msg_id), data_(data), len_(len), count_(0)
	{
	}
	void operator()(int id) {
		JmyTcpConnection* conn = mgr_.get(id);
		if (!conn) return;
		if (conn->send(msg_id_, data_, len_) > 0) {
			count_ += 1;	
		}
	}
	int get_count() const { return count_; }
private:
	JmyTcpConnectionMgr& mgr_;
	int msg_id_;
	const char* data_;
	unsigned int len_;
	int count_;
};

int JmyTcpClients::sendInturn(int msg_id, const char* data, unsigned int len)
{
	connection_send cs(mgr_, msg_id, data, len);
	std::for_each(used_ids_.begin(), used_ids_.end(), cs);
	return cs.get_count();
}

struct connection_run {
	connection_run(JmyTcpConnectionMgr& mgr) : mgr_(mgr), count_(0) {}
	void operator()(int id) {
		JmyTcpConnection* conn = mgr_.get(id);
		if (!conn) return;
		if (conn->run() >= 0) {
			count_ += 1;
		}
	}
	int get_count() const { return count_; }

private:
	JmyTcpConnectionMgr& mgr_;
	int count_;
};

int JmyTcpClients::runInturn()
{
	connection_run cr(mgr_);
	std::for_each(used_ids_.begin(), used_ids_.end(), cr);
	return cr.get_count();
}
