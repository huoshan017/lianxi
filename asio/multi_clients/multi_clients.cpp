#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <set>
#include <thread>
#include <chrono>
#include <iostream>
#if USE_CONNECTOR_AND_SESSION
#include "../libjmy/jmy_tcp_connector.h"
#include "../libjmy/jmy_tcp_session.h"
#else
#include "../libjmy/jmy_tcp_clients.h"
#endif
#include "const_data.h"
#include "util.h"

#define USE_ASYNC_CONNECT 0

using namespace boost::asio;

struct conn_data {
	int index_;
	bool send_failed_;
	conn_data(int index, bool send_failed) : index_(index), send_failed_(send_failed) {}
};

#if USE_CONNECTOR_AND_SESSION
static bool check_connected(JmyTcpMultiConnectors& connectors, int connector_id) {
	JmyConnState state = connectors.getState(connector_id);
#else
static bool check_connected(JmyTcpClients& clients, int conn_id) {
	JmyConnState state = clients.getState(conn_id);
#endif
	if (state == JMY_CONN_STATE_CONNECTED) {
		return true;
	}
	return false;
}

#if USE_CONNECTOR_AND_SESSION
static void connectors_run(io_service* service, int client_count)
#else
static void clients_run(io_service* service, int client_count)
#endif
{
#if USE_CONNECTOR_AND_SESSION
	JmyTcpMultiConnectors connectors(*service, client_count);
	ClientLogInfo("use_send_list(%d)", test_connector_config.common.use_send_list);
	if (!connectors.loadConfig(test_connector_config)) {
		ClientLogError("connector load config failed");
		return;
	}
#else
	JmyTcpClients clients(*service, client_count);
	if (!clients.loadConfig(test_clients_config)) {
		ClientLogError("clients load config failed");
		return;
	}
#endif
	
	std::map<int, conn_data> conns;
	for (int i=0; i<client_count; ++i) {
#if USE_CONNECTOR_AND_SESSION
		JmyTcpConnector* conn = connectors.start("127.0.0.1", 10000);
#else
		JmyTcpConnection* conn = clients.start();
#endif
		if (!conn) {
#if USE_CONNECTOR_AND_SESSION
			ClientLogError("connector(index:%d) start failed", i);
#else
			ClientLogError("connection(index:%d) start failed", i);
#endif
			return;
		}
#if USE_CONNECTOR_AND_SESSION
		ClientLogInfo("connector(%d) start, state %d", conn->getId(), conn->getState());
#else
		ClientLogInfo("client(%d) start, state %d", conn->getId(), conn->getConnState());
#endif
		conns.insert(std::make_pair(conn->getId(), conn_data(0, false)));
	}

	JmyConnState state = JMY_CONN_STATE_NOT_USE;
	uint64_t count = 0;
	int s = sizeof(s_send_data)/sizeof(s_send_data[0]);
	while (true) {
		std::map<int, conn_data>::iterator it = conns.begin();
		for (; it!=conns.end(); ++it) {
			int cid = it->first;
			if (state == JMY_CONN_STATE_NOT_CONNECT) {
#if USE_CONNECTOR_AND_SESSION
				if (!check_connected(connectors, cid)) {
#else
				if (!check_connected(clients, cid)) {
#endif
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
					continue;
				}
				state = JMY_CONN_STATE_CONNECTED;
			}
			if (!it->second.send_failed_) {
#if USE_CONNECTOR_AND_SESSION
				state = connectors.getState(cid);
#else
				state = clients.getState(cid);
#endif
				if (state != JMY_CONN_STATE_CONNECTED) {
					ClientLogInfo("client(%d) is not connected, state(%d)", cid, state);
					break;
				}
				int index = it->second.index_;
#if USE_CONNECTOR_AND_SESSION
				if (connectors.send(cid, 1, s_send_data[index], std::strlen(s_send_data[index])) < 0) {
#else
				if (clients.send(cid, 1, s_send_data[index], std::strlen(s_send_data[index])) < 0) {
#endif
					ClientLogWarn("connector send failed");
					it->second.send_failed_ = true;
					std::this_thread::sleep_for(std::chrono::seconds(1));
					continue;
				}
				it->second.index_ += 1;
				if (it->second.index_ >= s) {
					it->second.index_ = 0;
				}
				ClientLogDebug("connector(%d) send the %d str(%s) count %d", cid, index, s_send_data[index], count++);
			}
		}

#if USE_CONNECTOR_AND_SESSION
		if (connectors.runInturn() < 0) {
#else
		if (clients.runInturn() < 0) {
#endif
			ClientLogDebug("connector run failed");
			break;
		}
		size_t ss = service->poll();
		if (ss > 0) {
			//
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	} 
}

int main(int argc, char* argv[])
{
	static const char* s_log_conf = "./log.conf";
	if (!JmyLogInit(s_log_conf)) {
		std::cout << "load log config file " << s_log_conf << " failed" << std::endl;
		return -1;
	}
	if (!JmyLogOpenLib(s_libjmy_log_cate)) {
		std::cout << "open lib log " << s_libjmy_log_cate << " failed" << std::endl;
		return -1;
	}
	if (!JmyLogOpen(s_client_log_cate)) {
		std::cout << "open log " << s_client_log_cate << " failed" << std::endl;
		return -1;
	}

	if (argc < 2) {
		ClientLogError("argument parameter not enough");
		return -1;
	}

	(void)argv;
	io_service service;
	int client_count = (std::atoi(argv[1]));
	const int thread_count = 2;
	boost::thread_group ths;
	for (int i=0; i<thread_count; ++i) {
		int per_count = 0;
#if 1
		if (client_count % thread_count >= i + 1)
			per_count = client_count/thread_count + 1;
		else
			per_count = client_count/thread_count;
#endif
		if (per_count <= 0)
			break;
#if USE_CONNECTOR_AND_SESSION
		ths.create_thread(std::bind(connectors_run, &service, per_count));
#else
		ths.create_thread(std::bind(clients_run, &service, per_count));
#endif
		ClientLogInfo("thread run %d clients", per_count);
	}
	ths.join_all();
	return 0;
}
