#pragma once

#include <cstddef>

struct JmyBufferPoolData {
	unsigned int size;
	unsigned int count;
};

struct JmyBufferPoolDataList {
	JmyBufferPoolData* data;
	int ndata;
};

struct JmySessionConfig {
	unsigned int recv_buff_min;
	unsigned int recv_buff_max;
	unsigned int send_buff_min;
	unsigned int send_buff_max;
};

typedef int (*jmy_msg_handler)(const char*, unsigned int, int);

struct JmyId2MsgHandler {
	int msg_id;
	jmy_msg_handler handler;
};

struct JmyServerConfig {
	JmySessionConfig session_conf;
	JmyId2MsgHandler* handlers;
	int nhandlers;
	unsigned int max_conn;
};

struct JmyConnectorBaseConfig {
	unsigned int recv_buff_max_size;
	unsigned int send_buff_max_size;
};

struct JmyMultiSameConnectorsConfig {
	JmyConnectorBaseConfig base_conf;
	JmyId2MsgHandler* handlers;
	int nhandlers;
	unsigned int max_count;
	bool is_delay;
};

struct JmyConnectorConfig {
	JmyConnectorBaseConfig base;
	JmyId2MsgHandler* handlers;
	int nhandlers;
	bool is_delay;
};
