#include "http_request_mgr.h"
#include <thread>
#ifndef WIN32
#include <unistd.h>
#else
#include <windows.h>
#endif
#include <iostream>
#include <stdlib.h>
#include <stdint.h>
#include <atomic>
#include <chrono>

using namespace std;

#define USE_THREAD 1
#define USE_POST 0

static std::atomic<int> g_total;
static std::atomic<int> g_failed;
static std::atomic<int> g_success;

void callback_func(HttpResponse* response)
{
	if (response->error_code == 0) {
		g_success += 1;
	} else {
		g_failed += 1;
	}
	g_total += 1;
	cout << "total: " << g_total << ", success: " << g_success << ", data: " << response->data << ", len: " << response->len << ", userdata: " << response->user_data << endl;
}

int main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;
	if (argc < 3) {
		cout << "param num " << argc << " not enough" << endl;
		return -1;
	}

	const char* url = argv[1];

	int num = atoi(argv[2]);
	if (num <= 0) {
		cout << "param(" << num << ") invlaid" << endl;
		return -1;
	}

	g_total = 0;
	g_success = 0;
	g_failed = 0;
	HttpRequestMgr* mgr = new HttpRequestMgr;
	if (!mgr->init(num>5000?5000:num)) {
		cout << "HttpRequestMgr init failed" << endl;
		return -1;
	}
	mgr->setStatistics(true);

#if USE_THREAD
	mgr->use_thread(true);
	mgr->run();
#endif

	int i = 0;
	while (true) {
		// 请求多于HttpRequestPool中的HttpRequest数量
		if (i<num && mgr->hasReq()) {
#if USE_POST
			int res = mgr->post(url, s_post_content, callback_func, (void*)0);
#else
			int res = mgr->get(url, callback_func, (void*)0);
#endif
			if (res < 0) {
				return -1;
			}
			
			if (res == 0) {
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
				cout << "not enough free req" << endl;
			} else {
				i += 1;
			}
		}
#if !USE_THREAD
		if (mgr->run() < 0)
			break;
#endif
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	mgr->close();
	delete mgr;
	mgr = nullptr;

	return 0;
}
