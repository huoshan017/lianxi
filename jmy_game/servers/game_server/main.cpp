#include "game_server.h"

int main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;

	GameServer* server = new GameServer();
	if (!server->init()) {
		delete server;
		return -1;
	}
	int res = server->run();
	server->clear();
	delete server;
	return res;
}
