#include "../net_tcp/jmy_tcp_server.h"
#include "config_data.h"
#include <chrono>

#define INIT_PORT 0

int main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;
#if INIT_PORT
	JmyTcpServer server(10000);
#else
	JmyTcpServer server;
#endif
	if (!server.loadConfig(test_config)) {
		return -1;
	}
	
#if INIT_PORT
	if (!server.start()) {
#else
	if (!server.listenStart(10000)) {
#endif	
		return -1;
	}

	while (server.run() >= 0) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	server.close();
	return 0;
}
