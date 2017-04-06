#include "gate_server.h"

#define ServerConfPath "./gate_server.json"

int main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;

	GateServer* server = new GateServer();
	if (!server->init(ServerConfPath)) {
		delete server;
		return -1;
	}
	if (server->run() < 0) {
		delete server;
		return -1;
	}
	server->close();
	delete server;
	return 0;
}
