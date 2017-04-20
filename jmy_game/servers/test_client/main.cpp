#include "test_client.h"
#include <iostream>

static const char* TestClientConfPath = "./test_client.json";

int main(int argc, char** argv)
{
	(void)argc;
	(void)argv;
	if (!TEST_CLIENT->init(TestClientConfPath)) {
		std::cout << "test client init failed" << std::endl;
		return -1;
	}
	TEST_CLIENT->run();
	TEST_CLIENT->close();
	return 0;
}
