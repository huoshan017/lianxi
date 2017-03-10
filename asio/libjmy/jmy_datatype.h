#pragma once

#include <cstddef>
#include <memory>
#include <cstring>

struct JmyData {
	const char* data;
	unsigned int len;
};

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

struct JmyConnectorConfigCommon {
	unsigned int recv_buff_max_size;
	unsigned int send_buff_max_size;
	JmyId2MsgHandler* handlers;
	int nhandlers;
	bool no_delay;
	bool connected_start;
	bool use_send_list;
};

struct JmyMultiConnectorsConfig {
	JmyConnectorConfigCommon common;
	unsigned int max_count;
};

struct JmyConnectorConfig {
	JmyConnectorConfigCommon common;
	int conn_id;
	void assign(JmyMultiConnectorsConfig& conf) {
		common = conf.common;
	}
};

enum JmyBufferDropCondition {
	DropConditionImmediate			= 0x0001,
	DropConditionGreatBufferCount	= 0x0002,
	DropConditionGreatUsedBytes		= 0x0004,
	DropConditionTimeOut			= 0x0008,
	DropConditionManual				= 0x0010,
	DropConditionCount,
};
