#include "db_server.h"
#include <iostream>

#define ServerConfPath "./db_server.json"

int main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;

	if (!DB_SERVER->init(ServerConfPath)) {
		std::cout << "init server failed" << std::endl;
		return -1;
	}

	DB_SERVER->run();
	DB_SERVER->close();

	return 0;
}
