#include "gate_server.h"

#define ServerConfPath "./gate_server.json"

int main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;

	if (!GATE_SERVER->init(ServerConfPath)) {
		return -1;
	}
	GATE_SERVER->run();
	GATE_SERVER->close();
	return 0;
}
