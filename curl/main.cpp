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

//static const char* s_url = "http://116.228.6.174/0/login?appid=24&token=adsf";
static const char* s_url = "http://116.228.6.174:81/10/ZuanshiGain";
static const char* s_post_content = "{\"app_channel\":\"\",\"channel_id\":0,\"day\":1,\"group_id\":0,\"hour\":17,\"ip_online_num\":\"\",\"log_ym\":201606,\"log_ymd\":20160601,\"md5data\":\"3b6ead92ea7b400f493d8a74cb6c969c\",\"minute\":8,\"month\":6,\"online\":2,\"online_time\":1464772100,\"os_name\":\"debian\",\"platform_tag\":\"aofei\",\"server\":3,\"week\":23,\"year\":2016}";

#define USE_THREAD 1
#define USE_OLD 0

static std::atomic<int> g_total;
static std::atomic<int> g_failed;
static std::atomic<int> g_success;

#if !USE_RESPONSE_UNITY
int error_proc(int error, void* param)
{
	g_failed += 1;
	g_total += 1;
	cout << "g_total: " << g_total << ", g_failed: " << g_failed << ", error: " << error << ", param: " << param << endl;
	return 0;
}

void callback_func(char* ptr, size_t size, void* param)
{
	g_success += 1;
	g_total += 1;
	cout << "g_total: " << g_total << ", g_success: " << g_success << ", ptr: " << ptr << ", size: " << size << ", param: " << param << endl;
}
#else
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
#endif

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

	g_total = 0;
	g_success = 0;
	g_failed = 0;
	HttpRequestMgr* mgr = new HttpRequestMgr;
	if (!mgr->init(num>5000?5000:num)) {
		cout << "HttpRequestMgr init failed" << endl;
		return -1;
	}
	//mgr->setOutputDebug(true);

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

#if USE_RESPONSE_UNITY
			int res = mgr->post(s_url, s_post_content, callback_func, (void*)0);
#else
			int res = mgr->post(s_url, s_post_content, callback_func, (void*)0, error_proc, (void*)0);
#endif
			if (res < 0) {
				return -1;
			} else if (res == 0) {
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
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
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	mgr->close();
	delete mgr;
	mgr = NULL;

	return 0;
}
