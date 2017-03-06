#include "../libjmy/jmy_tcp_server.h"
#include "config_data.h"
#include <chrono>
#include <iostream>

#define INIT_PORT 0
#define LISTEN_PORT 10000

int main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;
#if INIT_PORT
	JmyTcpServer server(LISTEN_PORT);
#else
	JmyTcpServer server;
#endif
	if (!server.loadConfig(test_config)) {
		return -1;
	}
	
#if INIT_PORT
	if (!server.start()) {
#else
	if (!server.listenStart(LISTEN_PORT)) {
#endif	
		return -1;
	}

	std::cout << "start listening port " << LISTEN_PORT << std::endl;

	while (server.run() >= 0) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	server.close();
	return 0;
}
