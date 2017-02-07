#include "http_request_mgr.h"
#include <thread>
#include <unistd.h>
#include <iostream>
#include <stdlib.h>
#include <stdint.h>

using namespace std;

//static const char* s_url = "http://116.228.6.174/0/login?appid=24&token=adsf";
static const char* s_url = "192.168.0.200:80";

#define USE_THREAD 0

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

#if USE_THREAD
	mgr->thread_run();
#endif

	HttpResult* res = NULL;
	bool b = false;
	int i = 0;
	while (true) {
		// 请求多于HttpRequestPool中的HttpRequest数量
		if (i<num && mgr->hasFreeReq()) {
			HttpRequest* req = mgr->newReq();
			if (!req) {
				cout << "newReq failed" << endl;
				return -1;
			}

			//req->setPost(true);
			//req->setPostContent("&a=111&b=222&c=3");
			req->setGet(true);
			req->setUrl(s_url);
			req->setRespWriteFunc(HttpRequestMgr::write_callback, req);
			//req->setPrivate((void*)s_url);

			if (!mgr->addReq(req)) {
				cout << "add req failed" << endl;
				return -1;
			}

			i += 1;
		}

		if (mgr->one_loop() < 0)
			break;

		b = mgr->getResult(res);
		if (b) {
			//mgr->freeReq(res->req);
			mgr->freeResult(res);
		}
		usleep(100);
	}

#if USE_THREAD
	mgr->thread_join();
#endif
	mgr->close();

	return 0;
}
