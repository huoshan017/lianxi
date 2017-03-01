#include <boost/asio.hpp>
#include <chrono>
#include <iostream>
#include <cstring>
#include "../net_tcp/jmy_tcp_connector.h"
#include "config_data.h"

const char* s_send_data = "!@#$:1234567890abcdefghijklmnopqrstuvwxyz";

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
	JmyTcpConnector connector;
	if (!connector.loadConfig(test_connector_config)) {
		std::cout << "connector load config failed" << std::endl;
		return -1;
	}
	if (!connector.init(service)) {
		std::cout << "connector init failed" << std::endl;
		return -1;
	}
	connector.async_connect("127.0.0.1", port);
	JmyConnectorState state = CONNECTOR_STATE_NOT_CONNECT;
	while (true)  {
		service.poll();
		state = connector.getState();
		if (state == CONNECTOR_STATE_CONNETED) {
			std::cout << "connector connect port " << port << " success" << std::endl;
			connector.start();
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	std::cout << "connector enter to poll" << std::endl;
	connector.send(1, s_send_data, std::strlen(s_send_data));
	while (true) {
		if (connector.run() < 0) {
			std::cout << "connector run failed" << std::endl;
			break;
		}
		service.poll();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	} 

	return 0;
}
