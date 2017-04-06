#include "jmy_tcp_client.h"

JmyTcpClient::JmyTcpClient(JmyTcpConnection* conn) : conn_(conn)
{
}

JmyTcpClient::JmyTcpClient(JmyTcpConnection* conn, const ip::tcp::endpoint& ep) : conn_(conn), ep_(ep)
{
}

JmyTcpClient::~JmyTcpClient()
{
	destroy();
}

void JmyTcpClient::close()
{
	if (conn_)
		conn_->close();
}

void JmyTcpClient::destroy()
{
	if (conn_)
		conn_->destroy();
}

void JmyTcpClient::reset()
{
	if (conn_)
		conn_->reset();
}

bool JmyTcpClient::connect(const char* ip, unsigned short port, bool non_blocking)
{
	if (non_blocking) {
		conn_->asynConnect(ip, port);
	} else {
		return conn_->connect(ip, port);
	}
	return true;
}

bool JmyTcpClient::start(const JmyClientConfig& conf, bool non_blocking)
{
	std::shared_ptr<JmyDataHandler> data_handler = std::make_shared<JmyDataHandler>();
	if (conf.conn_conf.handlers && conf.conn_conf.nhandlers) {
		if (!data_handler->loadMsgHandle(conf.conn_conf.handlers, conf.conn_conf.nhandlers)) {
			return false;
		}
		conn_->setDataHandler(data_handler);
	}

	std::shared_ptr<JmyEventHandlerManager> event_handler = std::make_shared<JmyEventHandlerManager>();
	event_handler->setBaseHandlers(conf.conn_conf.base_event_handlers);
	if (conf.conn_conf.other_event_handlers && conf.conn_conf.other_event_nhandlers) {
		event_handler->init(conf.conn_conf.other_event_handlers, conf.conn_conf.other_event_nhandlers);
	}
	conn_->setEventHandler(event_handler);

	std::shared_ptr<JmyConnectionBuffer> buffer = std::make_shared<JmyConnectionBuffer>();
	buffer->init(conf.conn_conf.buff_conf);
	conn_->setBuffer(buffer);

	return connect(conf.conn_ip, conf.conn_port, non_blocking);
}

bool JmyTcpClient::reconnect(const JmyClientConfig& conf, bool non_blocking)
{
	if (conn_->getConnState() != JMY_CONN_STATE_NOT_CONNECT) {
		return false;
	}
	return connect(conf.conn_ip, conf.conn_port, non_blocking);
}

int JmyTcpClient::send(int msg_id, const char* data, unsigned int len)
{
	if (!conn_) return -1;
	return conn_->send(msg_id, data, len);
}

int JmyTcpClient::run()
{
	if (!conn_) return -1;
	if (isConnecting()) {
		return 0;
	} else if (isDisconnected()) {
		return -1;
	}
	return conn_->run();
}

/* JmyTcpClientMaster */
JmyTcpClientMaster::JmyTcpClientMaster(io_service& service) : conn_mgr_(service), max_count_(0), inited_(false)
{
}

JmyTcpClientMaster::~JmyTcpClientMaster()
{
	close();
}

bool JmyTcpClientMaster::init(int max_client_size)
{
	if (inited_) return true;
	conn_mgr_.init(max_client_size, JMY_CONN_TYPE_ACTIVE);
	max_count_ = max_client_size;
	inited_ = true;
	return true;
}

void JmyTcpClientMaster::close()
{
	inited_ = false;
	max_count_ = 0;
	used_ids_.clear();
	free_ids_.clear();
	id_gen_.reset();
	std::set<JmyTcpClient*>::iterator it = gene_clients_.begin();
	for (; it!=gene_clients_.end(); ++it) {
		if (*it) jmy_mem_free(*it);
	}
	gene_clients_.clear();
}

JmyTcpClient* JmyTcpClientMaster::generate()
{
	size_t s = used_ids_.size();
	if ((int)s >= max_count_) {
		LibJmyLogWarn("current connection count %d is max", (int)s);
		return 0;
	}

	JmyTcpConnection* conn = NULL;
	int id = 0;
	if (free_ids_.size() == 0) {
		id = id_gen_.get();
		conn = conn_mgr_.getFree(id);
		if (!conn) {
			LibJmyLogError("insert connection failed");
			return nullptr;
		}
		id = conn->getId();
		LibJmyLogInfo("connection id = %d", id);
	} else {
		id = free_ids_.front();
		conn = conn_mgr_.get(id);
		if (!conn) {
			LibJmyLogError("get connection(%d) failed", id);
			return nullptr;
		}
		conn->reset();
	}

	used_ids_.insert(id);
	JmyTcpClient* client = jmy_mem_malloc<JmyTcpClient>(conn);
	gene_clients_.insert(client);
	return client;
}

bool JmyTcpClientMaster::recycle(JmyTcpClient* client)
{
	std::set<JmyTcpClient*>::iterator it = gene_clients_.find(client);
	if (it == gene_clients_.end())
		return false;

	conn_mgr_.free(client->conn_);
	jmy_mem_free(client);
	gene_clients_.erase(client);

	return true;
}
