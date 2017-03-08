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

static bool check_connected(JmyTcpMultiConnectors& connectors, int connector_id)
{
	JmyConnectorState state = connectors.getState(connector_id);
	if (state == CONNECTOR_STATE_CONNECTED) {
		return true;
	}
	return false;
}

static void connectors_run(int client_count)
{
	io_service service;
	JmyTcpMultiConnectors connectors(service, client_count);
	if (!connectors.loadConfig(test_connector_config)) {
		ClientLogError("connector load config failed");
		return;
	}
	
	std::set<int> connector_ids;
	for (int i=0; i<client_count; ++i) {
		int cid = connectors.start("127.0.0.1", 10000);
		if (cid < 0) {
			ClientLogError("connector(index:%d) start failed", i);
			return;
		}
		if (cid == 0) {
			ClientLogDebug("connector(index:%d) start full", i);
			break;
		}
		connector_ids.insert(cid);
	}

	JmyConnectorState state = CONNECTOR_STATE_NOT_CONNECT;
	uint64_t count = 0;
	int i = 0;
	int s = sizeof(s_send_data)/sizeof(s_send_data[0]);
	bool send_failed = false;
	while (true) {
		size_t ss = service.poll();
		if (ss > 0) {
		}
		std::set<int>::iterator it = connector_ids.begin();
		for (; it!=connector_ids.end(); ++it) {
			int cid = *it;
			if (state == CONNECTOR_STATE_NOT_CONNECT) {
				if (!check_connected(connectors, cid)) {
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
					continue;
				}
				state = CONNECTOR_STATE_CONNECTED;
			}
			if (!send_failed) {
				if (connectors.getState(cid) != CONNECTOR_STATE_CONNECTED) {
					ClientLogDebug("connector is not connected");
					break;
				}
				int index = i % s;
				ClientLogDebug("connector send the %d str(%s) count %d", index, s_send_data[index], count++);
				if (connectors.send(cid, 1, s_send_data[index], std::strlen(s_send_data[index])) < 0) {
					ClientLogDebug("connector send failed");
					send_failed = true;
					std::this_thread::sleep_for(std::chrono::seconds(1));
					continue;
				}
				i += 1;
			}
		}
		if (connectors.runInturn() < 0) {
			ClientLogDebug("connector run failed");
			break;
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
	int client_count = (std::atoi(argv[1]));
	const int thread_count = 10;
	boost::thread_group ths;
	for (int i=0; i<thread_count; ++i) {
		int per_count = client_count/thread_count + (client_count%thread_count)/(i+1);
		if (per_count <= 0)
			break;
		ths.create_thread(std::bind(connectors_run, per_count));
		ClientLogDebug("thread run %d clients", per_count);
	}
	ths.join_all();
	return 0;
}
