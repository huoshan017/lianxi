#include "http_request_mgr.h"
#include <thread>
#include <unistd.h>
#include <iostream>
#include <stdlib.h>
#include <stdint.h>

using namespace std;

//static const char* s_url = "http://116.228.6.174/0/login?appid=24&token=adsf";
static const char* s_url = "192.168.3.250:80";


static void do_result(HttpRequestMgr* mgr)
{
	char* p = NULL;
	int len = 0;
	bool b = false;
	while (true) {
		b = mgr->getResult(p, len);
		if (b) {
			mgr->freeResult(p, len);
		}
		usleep(10000);
	}
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
		cout << "param(" << num << ") invlaid" << endl;
		return -1;
	}

	HttpRequestMgr* mgr = HttpRequestMgr::getInstance();
	if (!mgr->init(num>5000?5000:num)) {
		cout << "HttpRequestMgr init failed" << endl;
		return -1;
	}

	std::thread work_thread(HttpRequestMgr::thread_func, mgr);
	std::thread result_thread(do_result, mgr);

	int i = 0;
	for (; i<num; i++) {
		// 请求多于HttpRequestPool中的HttpRequest数量
		while (!mgr->hasFreeReq()) {
			usleep(100);
		}
		HttpRequest* req = mgr->newReq();
		if (!req) {
			cout << "newReq failed" << endl;
			return -1;
		}

		//req->setPost(true);
		//req->setPostContent("&a=111&b=222&c=3");
		req->setGet(true);
		req->setUrl(s_url);
		req->setPrivate((void*)s_url);

		if (!mgr->addReq(req)) {
			cout << "add req failed" << endl;
			return -1;
		}
	}

	work_thread.join();
	result_thread.join();

	mgr->close();

	return 0;
}
