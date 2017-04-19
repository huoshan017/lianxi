#include "test_client.h"
#include <iostream>

static const char* TestClientConfPath = "./test_client.json";

int main(int argc, char** argv)
{
	(void)argc;
	(void)argv;
	TestClient client;
	if (!client.init(TestClientConfPath)) {
		std::cout << "test client init failed" << std::endl;
		return -1;
	}
	client.run();
	client.close();
	return 0;
}
