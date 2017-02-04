#include "http_request_mgr.h"
#include <thread>
#include <unistd.h>
#include <iostream>
#include <stdlib.h>
#include <stdint.h>

using namespace std;

static const char* s_url = "192.168.0.110:80";

static size_t read_func(char* ptr, size_t size, size_t nmemb, void* userdata)
{
	(void)ptr;
	(void)userdata;
	size_t s = size*nmemb;
	cout << "do read_func" << endl;
	return s;
}

int main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;
	if (argc < 2) {
		cout << "param num(" << argc << ") invalid" << endl;
		return -1;
	}

	int num = atoi(argv[1]);
	if (num <= 0) {
		cout << "param(" << num << ")invlaid" << endl;
		return -1;
	}

	HttpRequestMgr* mgr = HttpRequestMgr::getInstance();
	if (!mgr->init(num>1500?1500:num)) {
		cout << "HttpRequestMgr init failed" << endl;
		return -1;
	}

	//mgr->run();
	std::thread work_thread(HttpRequestMgr::thread_func, mgr);

	// 初始化请求
	int i = 0;
	for (; i<num; i++) {
		while (!mgr->hasFreeReq()) {
			usleep(100);
		}
		HttpRequest* req = mgr->newReq();
		if (!req) {
			cout << "newReq failed" << endl;
			return -1;
		}
		req->setPost("&a=111&b=222&c=3");
		req->setUrl(s_url);
		req->setPrivate((void*)s_url);

		if (!mgr->addReq(req)) {
			cout << "add req failed" << endl;
			return -1;
		}
		//cout << "num=" << i << endl;
	}

	work_thread.join();

	return 0;
}
