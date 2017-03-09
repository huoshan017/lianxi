#include <boost/asio.hpp>
#include <chrono>
#include <iostream>
#include <cstring>
#include "../libjmy/jmy.h"
#include "const_data.h"
#include "util.h"

#define USE_ASYNC_CONNECT 1

using namespace boost::asio;

bool check_connected(JmyTcpConnector& connector)
{
	JmyConnectorState state = connector.getState();
	if (state == CONNECTOR_STATE_CONNECTED) {
		ClientLogDebug("connector connect port %d success", connector.getPort());
		if (!connector.isStarting()) {
			connector.start();
			ClientLogInfo("connector starting");
		}
		return true;
	}
	return false;
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
	short port = (short)(std::atoi(argv[1]));

	io_service service;
	JmyTcpConnector connector(service);
	if (!connector.loadConfig(test_connector_config)) {
		ClientLogError("connector load config failed");
		return -1;
	}

#if !USE_ASYNC_CONNECT
	connector.connect("127.0.0.1", port);
	connector.start();
#else
	connector.asynConnect("127.0.0.1", port);
#endif

	JmyConnectorState state = CONNECTOR_STATE_NOT_CONNECT;
	uint64_t count = 0;
	int i = 0;
	int s = sizeof(s_send_data)/sizeof(s_send_data[0]);
	bool send_failed = false;
	while (true) {
		size_t ss = service.poll();
		if (ss > 0) {
		}
		if (state == CONNECTOR_STATE_NOT_CONNECT) {
			if (!check_connected(connector)) {
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				continue;
			}
			state = CONNECTOR_STATE_CONNECTED;
		}
		if (!send_failed) {
			if (connector.getState() != CONNECTOR_STATE_CONNECTED) {
				ClientLogDebug("connector is not connected");
				break;
			}
			int index = i % s;
			ClientLogDebug("connector send the %d str(%s) count %d", index, s_send_data[index], count++);
			if (connector.send(1, s_send_data[index], std::strlen(s_send_data[index])) < 0) {
				ClientLogDebug("connector send failed");
				send_failed = true;
				std::this_thread::sleep_for(std::chrono::seconds(1));
				continue;
			}
			i += 1;
		}

		if (connector.run() < 0) {
			ClientLogDebug("connector run failed");
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	} 

	return 0;
}
