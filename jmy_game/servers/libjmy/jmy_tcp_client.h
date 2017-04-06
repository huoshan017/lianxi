#pragma once
#include "jmy_tcp_connection.h"
#include "jmy_connection_buffer.h"
#include "jmy_data_handler.h"
#include "jmy_event_handler.h"
#include "jmy_util.h"

class JmyTcpClient
{
public:
	JmyTcpClient(JmyTcpConnection* conn);
	JmyTcpClient(JmyTcpConnection* conn, const ip::tcp::endpoint& ep);
	~JmyTcpClient();

	void close();
	void destroy();
	void reset();

	bool start(const JmyClientConfig& conf, bool non_blocking = true);
	bool reconnect(const JmyClientConfig& conf, bool no_blocking = true);
	int send(int msg_id, const char* data, unsigned int len);
	int run();

	JmyConnState getState() const { return conn_->getConnState(); }
	bool isNotConnect() const { return conn_->getConnState() == JMY_CONN_STATE_NOT_CONNECT; }
	bool isConnected() const { return conn_->isConnected(); }
	bool isDisconnected() const { return conn_->isDisconnect(); }
	bool isConnecting() const { return conn_->getConnState() == JMY_CONN_STATE_CONNECTING; }

private:
	bool connect(const char* ip, unsigned short port, bool non_blocking = true);

private:
	JmyTcpConnection* conn_;
	ip::tcp::endpoint ep_;

	friend class JmyTcpClientMaster;
};

class JmyTcpClientMaster
{
public:
	JmyTcpClientMaster(io_service& service);
	~JmyTcpClientMaster();

	void close();
	bool init(int max_client_size);
	JmyTcpClient* generate();
	bool recycle(JmyTcpClient*);

private:
	JmyTcpConnectionMgr conn_mgr_;
	std::set<JmyTcpClient*> gene_clients_;
	JmyIdGenerator<int> id_gen_;
	std::set<int> used_ids_;
	std::list<int> free_ids_;
	int max_count_;
	bool inited_;
};
