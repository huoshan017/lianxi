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
static const char* s_url = "http://116.228.6.174:81/10/ZuanshiGain";
static const char* s_post_content = "{\"app_channel\":\"\",\"channel_id\":0,\"day\":1,\"group_id\":0,\"hour\":17,\"ip_online_num\":\"\",\"log_ym\":201606,\"log_ymd\":20160601,\"md5data\":\"3b6ead92ea7b400f493d8a74cb6c969c\",\"minute\":8,\"month\":6,\"online\":2,\"online_time\":1464772100,\"os_name\":\"debian\",\"platform_tag\":\"aofei\",\"server\":3,\"week\":23,\"year\":2016}";

#define USE_THREAD 1
#define USE_OLD 1

int error_proc(int error, void* param)
{
	cout << "error: " << error << ", param: " << param << endl;
	return 0;
}

void callback_func(char* ptr, size_t size, void* param)
{
	(void)ptr;
	cout << "ptr: " << ptr << ", size: " << size << ", param: " << param << endl;
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
	mgr->setOutputDebug(true);

#if USE_THREAD
	mgr->use_thread(true);
	mgr->run();
#endif

	int i = 0;
	while (true) {
		// 请求多于HttpRequestPool中的HttpRequest数量
		if (i<num && mgr->hasFreeReq()) {
#if USE_OLD
			HttpRequest* req = mgr->newReq();
			if (!req) {
				cout << "newReq failed" << endl;
				return -1;
			}

			req->setUrl(s_url);
			req->setErrorFunc(error_proc, (void*)req);
			//req->setGet(true);
			req->setPost(true);
			req->setPostContent(s_post_content);
			
			req->setRespWriteFunc(callback_func, req);
			//req->setPrivate((void*)s_url);

			if (!mgr->addReq(req)) {
				cout << "add req failed" << endl;
				return -1;
			}
			i += 1;
#else
			int res = mgr->post(s_url, s_post_content, callback_func, (void*)0, error_proc, (void*)0);
			if (res < 0) {
				return -1;
			} else if (res == 0) {
#ifndef WIN32
				usleep(100);
#else
				::Sleep(1);
#endif
				cout << "not enough free req" << endl;
			} else {
				i += 1;
			}
#endif
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
