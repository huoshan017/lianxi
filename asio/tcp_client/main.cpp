#include <boost/asio.hpp>
#include <chrono>
#include <iostream>
#include <cstring>
#include "../libjmy/jmy_tcp_connector.h"
#include "../libjmy/jmy_net_tool.h"
#include "config_data.h"
#include "const_data.h"

#define USE_ASYNC_CONNECT 1

using namespace boost::asio;

bool check_connected(JmyTcpConnector& connector)
{
	JmyConnectorState state = connector.getState();
	if (state == CONNECTOR_STATE_CONNECTED) {
		std::cout << "connector connect port " << connector.getPort() << " success" << std::endl;
		if (!connector.isStarting()) {
			connector.start();
			std::cout << "connector starting" << std::endl;
		}
		return true;
	}
	return false;
}

int main(int argc, char* argv[])
{
	if (argc < 2) {
		std::cout << "argument parameter not enough" << std::endl;
		return -1;
	}

	(void)argv;
	short port = (short)(std::atoi(argv[1]));

	io_service service;
	JmyTcpConnector connector(service);
	if (!connector.loadConfig(test_connector_config)) {
		std::cout << "connector load config failed" << std::endl;
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
				std::cout << "connector is not connected" << std::endl;
				break;
			}
			int index = i % s;
			std::cout << "connector send the " << index << " str(" << s_send_data[index] << ") count " << count++ << std::endl;
			if (connector.send(1, s_send_data[index], std::strlen(s_send_data[index])) < 0) {
				std::cout << "connector send failed" << std::endl;
				send_failed = true;
				std::this_thread::sleep_for(std::chrono::seconds(1));
				continue;
			}
			i += 1;
		}

		if (connector.run() < 0) {
			std::cout << "connector run failed" << std::endl;
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	} 

	return 0;
}
