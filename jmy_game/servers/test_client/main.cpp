#include "test_client.h"
#include "../common/util.h"
#include "config_loader.h"
#include <cstdlib>
#include <thread>

static const char* TestClientConfPath = "./test_client.json";

int main(int argc, char** argv)
{
	(void)argc;
	(void)argv;
	if (!CLIENT_MGR->init(TestClientConfPath)) {
		LogError("test client init failed"); 
		return -1;
	}

	int i = 0;
	int s = CLIENT_CONFIG.account_num;
	for (; i<s; ++i) {
		std::string account = CLIENT_CONFIG.account_prefix + std::to_string(CLIENT_CONFIG.account_start_index+i);
		if (!CLIENT_MGR->startClient(account)) {
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	CLIENT_MGR->run();
	CLIENT_MGR->clear();
	return 0;
}
