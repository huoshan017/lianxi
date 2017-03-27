#include "../../libjmy/jmy_tcp_server.h"

static const short s_listen_port = 10000;

int main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;
	JmyTcpServer server;
	//server.loadConfig();
	if (server.listenStart(s_listen_port)) {
		return -1;
	}
	return 0;
}
