#include <boost/asio.hpp>
#include <chrono>
#include <iostream>
#include <cstring>
#include "../libjmy/jmy_tcp_connector.h"
#include "../libjmy/jmy_net_tool.h"
#include "config_data.h"
#include "const_data.h"

#define USE_ASYNC_CONNECT 0

using namespace boost::asio;
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
	connector.async_connect("127.0.0.1", port);
	JmyConnectorState state = CONNECTOR_STATE_NOT_CONNECT;
	while (true)  {
		size_t s = service.poll();
		if (s > 0) {
			std::cout << "connector service polled " << s << " to wait connect server" << std::endl;
		}
		state = connector.getState();
		if (state == CONNECTOR_STATE_CONNECTED) {
			std::cout << "connector connect port " << port << " success" << std::endl;
			//connector.start();
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
#endif

	JmyNetTool nt;
	nt.start();
	uint64_t count = 0;
	int i = 0;
	int s = sizeof(s_send_data)/sizeof(s_send_data[0]);
	bool send_failed = false;
	while (true) {
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
			nt.addUpStream(std::strlen(s_send_data[index]));
			i += 1;
		}
		if (connector.run() < 0) {
			std::cout << "connector run failed" << std::endl;
			break;
		}
		size_t s = service.poll();
		if (s > 0) {
		}
		std::cout << "connector send bps: " << nt.getSendBps() << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	} 

	return 0;
}
