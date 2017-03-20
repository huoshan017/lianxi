#pragma once

#include "jmy_datatype.h"
#include "jmy_tcp_connection.h"
#include "jmy_connection_buffer.h"
#include "jmy_data_handler.h"
#include "jmy_util.h"
#include <set>
#include <list>

enum { JMY_TCP_CLIENTS_MAX_COUNT = 5000, };

class JmyTcpClients
{
public:
	JmyTcpClients(io_service& service, int max_count = JMY_TCP_CLIENTS_MAX_COUNT);
	JmyTcpClients(io_service& service, const ip::tcp::endpoint& ep, int max_count = JMY_TCP_CLIENTS_MAX_COUNT);
	~JmyTcpClients();

	void close();
	void destroy();
	void reset();

	bool loadConfig(const JmyClientsConfig& conf);
	JmyTcpConnection* start();
	int send(int conn_id, int msg_id, const char* data, unsigned int len);
	int run(int conn_id);
	int startInturn(int count);
	int sendInturn(int msg_id, const char* data, unsigned int len);
	int runInturn();

	JmyTcpConnection* getConnection(int conn_id);
	JmyConnState getState(int conn_id);

private:
	int max_count_;
	std::set<int> used_ids_;
	std::list<int> free_ids_;
	ip::tcp::endpoint ep_;
	JmyClientsConfig conf_;
	JmyTcpConnectionMgr mgr_;
	JmyConnectionBufferMgr buffer_mgr_;
	JmyIdGenerator<int> id_gen_;
	std::shared_ptr<JmyDataHandler> handler_;
	std::shared_ptr<JmySessionBufferPool> buff_pool_;
};
