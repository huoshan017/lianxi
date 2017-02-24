#pragma once

#include <cstddef>

typedef int (*my_msg_handler)(const char*, int, int);

struct MyId2MsgHandler {
	int msg_id;
	my_msg_handler handler;
};

struct MySessionConfig {
	unsigned int recv_buff_min;
	unsigned int recv_buff_max;
	unsigned int send_buff_min;
	unsigned int send_buff_max;
	MyId2MsgHandler* handlers;
	int nhandlers;
	MySessionConfig() : recv_buff_min(0), recv_buff_max(0), send_buff_min(0), send_buff_max(0), handlers(NULL), nhandlers(0) {}
};

struct MyServerConfig {
	MySessionConfig session_conf;
	MyId2MsgHandler* handlers;
	int nhandlers;
	MyServerConfig() : handlers(NULL), nhandlers(0) {}
};
