#pragma once

#include <cstddef>
#include <memory>
#include <cstring>
#include "jmy_const.h"

struct JmyData {
	const char* data;
	unsigned int len;
	JmyData() : data(nullptr), len(0) {}
	JmyData(const char* d, unsigned int l) : data(d), len(l) {}
};

struct JmySessionInfo {
	JmyConnType type;
	int session_id;
};

struct JmyMsgInfo {
	int msg_id;
	char* data;
	unsigned int len;
	int session_id;
	void* param;
	JmyMsgInfo() : msg_id(0), data(nullptr), len(0), session_id(0), param(nullptr) {}
	JmyMsgInfo(int mid, char* d, unsigned int l, int sid, void* p) : msg_id(mid), data(d), len(l), session_id(sid), param(p) {}
};

typedef int (*jmy_msg_handler)(JmyMsgInfo*);

struct JmyId2MsgHandler {
	int msg_id;
	jmy_msg_handler handler;
};

// hold ack_count and curr_id
struct JmyAckInfo {
	unsigned short ack_count;
	unsigned short curr_id;
};

struct JmyAckMsgInfo {
	int session_id;
	void* session_param;
	JmyAckInfo ack_info;
};

struct JmyHeartbeatMsgInfo {
	int session_id;
	void* session_param;
};

#if USE_CONN_PROTO
// connect result to hold conn_id and session_str
struct JmyConnResInfo {
	unsigned int conn_id;
	char* session_str;
	unsigned char session_str_len;
};

struct JmyConnMsgInfo {
	int session_id;
	void* session_param;
};

struct JmyConnResMsgInfo {
	int session_id;
	void* session_param;
	JmyConnResInfo info;
};

// reconn info
struct JmyReconnMsgInfo {
	int session_id;
	void* session_param;
	JmyConnResInfo info;
};

// ack reconn info
struct JmyReconnResMsgInfo {
	int session_id;
	void* session_param;
	JmyConnResInfo new_info;
};
#endif

// configure for reconnect
struct JmyReconnectConfig {
	unsigned short max_cached_send_count;	// if the size of send messages great to the value, that is meant network has problem
	unsigned short ack_recv_count;			// acknowlege the count of receiving messages
};

// configure for session
struct JmySessionConfig {
	unsigned int recv_buff_min;
	unsigned int recv_buff_max;
	unsigned int send_buff_min;
	unsigned int send_buff_max;
	bool use_send_list;
	JmyReconnectConfig reconn_conf;
};

// configure for server
struct JmyServerConfig {
	JmySessionConfig session_conf;
	JmyId2MsgHandler* handlers;
	int nhandlers;
	unsigned int max_conn;
};

// configure for common connector
struct JmyConnectorConfigCommon {
	unsigned int recv_buff_max_size;
	unsigned int send_buff_max_size;
	JmyId2MsgHandler* handlers;
	int nhandlers;
	bool no_delay;
	bool connected_start;
	bool use_send_list;
	JmyReconnectConfig reconn_conf;
};

// configure for multiconnector
struct JmyMultiConnectorsConfig {
	JmyConnectorConfigCommon common;
	unsigned int max_count;
};

// configure for connector
struct JmyConnectorConfig {
	JmyConnectorConfigCommon common;
	int conn_id;
	void assign(JmyMultiConnectorsConfig& conf) {
		common = conf.common;
	}
};

struct JmyTotalReconnInfo {
#if USE_CONN_PROTO
	JmyConnResInfo conn_info;		// connect need info
#endif
	JmyAckInfo recv_info, send_info;	// transfer data need info
};
