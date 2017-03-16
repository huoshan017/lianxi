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
	JmySessionType type;
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

struct JmyAckInfo {
	unsigned short ack_count;
	unsigned short curr_id;
};

struct JmyAckMsgInfo {
	int session_id;
	void* session_param;
	JmyAckInfo ack_info;
};

// heartbeat info
struct JmyHeartbeatInfo {
};

struct JmyHeartbeatMsgInfo {
	int session_id;
	void* session_param;
	JmyHeartbeatInfo info;
};

// conn info 
struct JmyConnInfo {
};

struct JmyConnMsgInfo {
	int session_id;
	void* session_param;
	JmyConnInfo info;
};

// ack conn info
struct JmyAckConnInfo {
	unsigned int conn_id;
	char* session_str;
	unsigned char session_str_len;
};

struct JmyAckConnMsgInfo {
	int session_id;
	void* session_param;
	JmyAckConnInfo info;
};

// reconn info
struct JmyReconnInfo {
	unsigned int conn_id;
	char* session_str;
	unsigned char session_str_len;
};

struct JmyReconnMsgInfo {
	int session_id;
	void* session_param;
	JmyReconnInfo info;
};

// ack reconn info
struct JmyAckReconnInfo {
	unsigned int conn_id;
	char* session_str;
	unsigned char session_str_len;
};

struct JmyAckReconnMsgInfo {
	int session_id;
	void* session_param;
	JmyAckReconnInfo info;
};

// configure for reconnect
struct JmyReconnectConfig {
	unsigned short max_cached_send_count;	// if the size of send messages great to the value, that is meant network has problem
	unsigned short ack_recv_count;			// acknowlege the count of receiving messages
	//JmyReconnectConfig() : max_cached_send_count(0), ack_recv_count(0) {}
};

// configure for session
struct JmySessionConfig {
	unsigned int recv_buff_min;
	unsigned int recv_buff_max;
	unsigned int send_buff_min;
	unsigned int send_buff_max;
	bool use_send_list;
	JmyReconnectConfig reconn_conf;
	//JmySessionConfig() : recv_buff_min(0), recv_buff_max(0), send_buff_min(0), send_buff_max(0), use_send_list(false) {}
	//JmySessionConfig(unsigned int recv_min, unsigned int recv_max, unsigned int send_min, unsigned int send_max, bool use_list) 
	//	: recv_buff_min(recv_min), recv_buff_max(recv_max), send_buff_min(send_min), send_buff_max(send_max), use_send_list(use_list)
	//{}
};

// configure for server
struct JmyServerConfig {
	JmySessionConfig session_conf;
	JmyId2MsgHandler* handlers;
	int nhandlers;
	unsigned int max_conn;
	//JmyServerConfig() : handlers(nullptr), nhandlers(0), max_conn(0) {}
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
	//JmyConnectorConfigCommon()
	//	: recv_buff_max_size(0), send_buff_max_size(0), handlers(nullptr), nhandlers(0), no_delay(false), connected_start(false), use_send_list(false)
	//{}
};

// configure for multiconnector
struct JmyMultiConnectorsConfig {
	JmyConnectorConfigCommon common;
	unsigned int max_count;
	//JmyMultiConnectorsConfig() : max_count(0) {}
};

// configure for connector
struct JmyConnectorConfig {
	JmyConnectorConfigCommon common;
	int conn_id;
	//JmyConnectorConfig() : conn_id(0) {}
	void assign(JmyMultiConnectorsConfig& conf) {
		common = conf.common;
	}
};

enum JmyBufferDropCondition {
	//DropConditionImmediate			= 0x0001,
	DropConditionGreatBufferCount	= 0x0002,
	DropConditionGreatUsedBytes		= 0x0004,
	DropConditionTimeOut			= 0x0008,
	DropConditionManual				= 0x0010,
	DropConditionCount,
};

struct JmyReconnectInfo {
	unsigned short ack_send_msg_count; // use for send buffer
	unsigned short ack_recv_msg_count; // use for recv buffer
	unsigned int curr_ack_send_id;
	unsigned int curr_ack_recv_id;
	//JmyReconnectInfo() : ack_send_msg_count(0), ack_recv_msg_count(0), curr_ack_send_id(0), curr_ack_recv_id(0) {}
};
