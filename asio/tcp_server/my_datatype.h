#pragma once

#include <cstddef>

struct MySessionConfig {
	unsigned int recv_buff_min;
	unsigned int recv_buff_max;
	unsigned int send_buff_min;
	unsigned int send_buff_max;
};

typedef int (*my_msg_handler)(const char*, unsigned int, int);

struct MyId2MsgHandler {
	int msg_id;
	my_msg_handler handler;
};

struct MyServerConfig {
	MySessionConfig session_conf;
	MyId2MsgHandler* handlers;
	int nhandlers;
	unsigned int max_conn;
};
