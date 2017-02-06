#include "http_request_mgr.h"
#include <thread>
#include <unistd.h>
#include <iostream>
#include <stdlib.h>
#include <stdint.h>

using namespace std;

//static const char* s_url = "http://192.168.3.33:1024/api/serverlist?platform_id=xingluo";
static const char* s_url = "192.168.0.200:80";

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

	// 初始化请求
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
		//req->setPrivate((void*)s_url);

		if (!mgr->addReq(req)) {
			cout << "add req failed" << endl;
			return -1;
		}
	}

	work_thread.join();

	return 0;
}
