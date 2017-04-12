#include "db_server.h"
#include <iostream>

#define ServerConfPath "./db_server.json"

int main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;

	DBServer server;
	if (!server.init(ServerConfPath)) {
		std::cout << "init server failed" << std::endl;
		return -1;
	}

	server.run();
	server.close();

	return 0;
}
