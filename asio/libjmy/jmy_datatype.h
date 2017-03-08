#pragma once

#include <cstddef>
#include <memory>

struct JmyMsgInfo {
	int msg_id;
	char* data;
	unsigned int len;
	int session_id;
	void* param;

	JmyMsgInfo() : msg_id(0), data(NULL), len(0), session_id(0), param(NULL) {}
};

typedef int (*jmy_msg_handler)(JmyMsgInfo*);

struct JmyId2MsgHandler {
	int msg_id;
	jmy_msg_handler handler;
};

struct JmySessionConfig {
	unsigned int recv_buff_min;
	unsigned int recv_buff_max;
	unsigned int send_buff_min;
	unsigned int send_buff_max;
};

struct JmyServerConfig {
	JmySessionConfig session_conf;
	JmyId2MsgHandler* handlers;
	int nhandlers;
	unsigned int max_conn;
};

struct JmyConnectorConfig {
	unsigned int recv_buff_max_size;
	unsigned int send_buff_max_size;
	JmyId2MsgHandler* handlers;
	int nhandlers;
	bool no_delay;
	bool connected_start;
};

struct JmyMultiConnectorsConfig {
	JmyConnectorConfig config;
	unsigned int max_count;
};
