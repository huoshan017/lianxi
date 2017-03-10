#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <set>
#include <thread>
#include <chrono>
#include <iostream>
#include "../libjmy/jmy.h"
#include "const_data.h"
#include "util.h"

#define USE_ASYNC_CONNECT 0

using namespace boost::asio;

struct conn_data {
	int index_;
	bool send_failed_;
	conn_data(int index, bool send_failed) : index_(index), send_failed_(send_failed) {}
};

static bool check_connected(JmyTcpMultiConnectors& connectors, int connector_id)
{
	JmyConnectorState state = connectors.getState(connector_id);
	if (state == CONNECTOR_STATE_CONNECTED) {
		return true;
	}
	return false;
}

static void connectors_run(io_service* service, int client_count)
{
	JmyTcpMultiConnectors connectors(*service, client_count);
	ClientLogInfo("use_send_list(%d)", test_connector_config.common.use_send_list);
	if (!connectors.loadConfig(test_connector_config)) {
		ClientLogError("connector load config failed");
		return;
	}
	
	std::map<int, conn_data> conns;
	for (int i=0; i<client_count; ++i) {
		JmyTcpConnector* conn = connectors.start("127.0.0.1", 10000);
		if (!conn) {
			ClientLogError("connector(index:%d) start failed", i);
			return;
		}
		ClientLogInfo("connector(%d) start, state %d", conn->getId(), conn->getState());
		conns.insert(std::make_pair(conn->getId(), conn_data(0, false)));
	}

	JmyConnectorState state = CONNECTOR_STATE_NOT_CONNECT;
	uint64_t count = 0;
	int s = sizeof(s_send_data)/sizeof(s_send_data[0]);
	while (true) {
		std::map<int, conn_data>::iterator it = conns.begin();
		for (; it!=conns.end(); ++it) {
			int cid = it->first;
#if 1
			if (state == CONNECTOR_STATE_NOT_CONNECT) {
				if (!check_connected(connectors, cid)) {
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
					continue;
				}
				state = CONNECTOR_STATE_CONNECTED;
			}
#endif
			if (!it->second.send_failed_) {
				state = connectors.getState(cid);
				if (state != CONNECTOR_STATE_CONNECTED) {
					ClientLogInfo("connector(%d) is not connected, state(%d)", cid, state);
					break;
				}
				int index = it->second.index_;
				if (connectors.send(cid, 1, s_send_data[index], std::strlen(s_send_data[index])) < 0) {
					ClientLogWarn("connector send failed");
					it->second.send_failed_ = true;
					std::this_thread::sleep_for(std::chrono::seconds(1));
					continue;
				}
				it->second.index_ += 1;
				if (it->second.index_ >= s) {
					it->second.index_ = 0;
				}
				ClientLogInfo("connector(%d) send the %d str(%s) count %d", cid, index, s_send_data[index], count++);
			}
		}
		if (connectors.runInturn() < 0) {
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
	const int thread_count = 10;
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
		ths.create_thread(std::bind(connectors_run, &service, per_count));
		ClientLogInfo("thread run %d clients", per_count);
	}
	ths.join_all();
	return 0;
}
