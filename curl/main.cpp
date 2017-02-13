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

using namespace std;

//static const char* s_url = "http://116.228.6.174/0/login?appid=24&token=adsf";
static const char* s_url = "192.168.0.200:80";

#define USE_THREAD 1

int error_proc(int error, void* param)
{
	cout << "error: " << error << ", param: " << param << endl;
	return 0;
}

void callback_func(char* ptr, size_t size, void* param)
{
	(void)ptr;
	//cout << "size: " << size << ", param: " << param << endl;
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

#if USE_THREAD
	mgr->use_thread(true);
	mgr->run();
#endif

	int i = 0;
	while (true) {
		// 请求多于HttpRequestPool中的HttpRequest数量
		if (i<num && mgr->hasFreeReq()) {
			HttpRequest* req = mgr->newReq();
			if (!req) {
				cout << "newReq failed" << endl;
				return -1;
			}

			req->setUrl(s_url);
			req->setGet(true);
			req->setErrorFunc(error_proc, (void*)req);
			//req->setPost(true);
			//req->setPostContent("&a=111&b=222&c=3");
			
			req->setRespWriteFunc(callback_func, req);
			//req->setPrivate((void*)s_url);

			if (!mgr->addReq(req)) {
				cout << "add req failed" << endl;
				return -1;
			}

			i += 1;
		}
#if !USE_THREAD
		if (mgr->run() < 0)
			break;
#endif

#ifndef WIN32
		usleep(100);
#else
		::Sleep(1);
#endif
	}

	mgr->close();

	return 0;
}
