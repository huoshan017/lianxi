#include "login_server.h"

#define ServerConfPath "./login_server.json"

int main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;

	int res = 0;
	LoginServer* server = new LoginServer();
	if (server->init(ServerConfPath)) {
		res = server->run();
	} else {
		std::cout << "server init failed" << std::endl;
		res = -1;
	}
	server->clear();
	delete server;
	return res;
}
