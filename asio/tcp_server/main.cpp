#include "../libjmy/jmy_tcp_server.h"
#include "util.h"
#include <chrono>
#include <iostream>

#define INIT_PORT 0
#define LISTEN_PORT 10000
#define LogConf "./log.conf"

int main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;
	if (!JmyLogInit(LogConf)) {
		std::cout << "failed to init log file: " << LogConf << std::endl;
		return -1;
	}
	if (!JmyLogOpenLib(s_libjmy_log_cate)) {
		std::cout << "failed to create lib log category: " << s_libjmy_log_cate << std::endl;
		return -1;
	}
	if (!JmyLogOpen(s_server_log_cate)) {
		std::cout << "failed to create server log category: " << s_server_log_cate << std::endl;
		return -1;
	}
#if INIT_PORT
	JmyTcpServer server(LISTEN_PORT);
#else
	JmyTcpServer server;
#endif
	if (!server.loadConfig(test_config)) {
		return -1;
	}
	
#if INIT_PORT
	if (server.start() < 0) {
#else
	if (server.listenStart(LISTEN_PORT) < 0) {
#endif	
		return -1;
	}

	ServerLogInfo("start listening port %d", LISTEN_PORT);

	while (server.run() >= 0) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	server.close();
	return 0;
}
