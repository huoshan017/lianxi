#pragma once

#include <cstddef>
#include <memory>
#include <cstring>
#include "jmy_const.h"

#define USE_CONNECTOR_AND_SESSION 0
#define USE_NET_PROTO2 1

struct JmyData {
	const char* data;
	unsigned int len;
	JmyData() : data(nullptr), len(0) {}
	JmyData(const char* d, unsigned int l) : data(d), len(l) {}
};

struct JmyMsgInfo {
	int user_id;
	int msg_id;
	char* data;
	unsigned int len;
	int conn_id;
	void* param;
	JmyMsgInfo() : user_id(0), msg_id(0), data(nullptr), len(0), conn_id(0), param(nullptr) {}
	JmyMsgInfo(int rid, int mid, char* d, unsigned int l, int sid, void* p) : user_id(rid), msg_id(mid), data(d), len(l), conn_id(sid), param(p) {}
};

typedef int (*jmy_msg_handler)(JmyMsgInfo*);

struct JmyId2MsgHandler {
	int msg_id;
	jmy_msg_handler handler;
};

struct JmyEventInfo {
	int event_id;
	int conn_id;
	void* param;
	long param_l;
	JmyEventInfo() : event_id(0), conn_id(0), param(nullptr), param_l(0) {}
	JmyEventInfo(int eid, int cid, void* p, long p2) : event_id(eid), conn_id(cid), param(p), param_l(p2) {}
};

typedef int (*jmy_event_handler)(JmyEventInfo*);

struct JmyBaseEventHandlers {
	jmy_event_handler conn_handler;
	jmy_event_handler disconn_handler;
	jmy_event_handler tick_handler;
	jmy_event_handler timer_handler;
};

struct JmyId2EventHandler {
	int event_id;
	jmy_event_handler handler;
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

struct JmyDisconnectMsgInfo {
	int session_id;
	void* session_param;
};

struct JmyDisconnectAckMsgInfo {
	int session_id;
	void* session_param;
};

// buffer config
struct JmyBufferConfig {
	unsigned int recv_buff_size;
	unsigned int send_buff_size;
	unsigned int recv_buff_list_size;
	unsigned int send_buff_list_size;
	bool use_recv_buff_list;
	bool use_send_buff_list;
};

// connection configure
struct JmyConnectionConfig {
	JmyBufferConfig buff_conf;
	JmyId2MsgHandler* handlers;
	int nhandlers;
	jmy_msg_handler default_msg_handler;
	JmyBaseEventHandlers base_event_handlers;
	JmyId2EventHandler* other_event_handlers;
	int other_event_nhandlers;
	bool no_delay;
};

#if USE_CONNECTOR_AND_SESSION
// configure for session
struct JmySessionConfig {
	unsigned int recv_buff_min;
	unsigned int recv_buff_max;
	unsigned int send_buff_min;
	unsigned int send_buff_max;
	bool use_send_list;
	JmyReconnectConfig reconn_conf;
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
#else
// configure for client
struct JmyClientConfig {
	JmyConnectionConfig conn_conf;
	bool is_reconnect;
	int reconnect_interval;
};

// configure for clients
struct JmyClientsConfig {
	JmyConnectionConfig conn_conf;
	char* conn_ip;
	unsigned short conn_port;
	unsigned int max_conn;
};
#endif

// configure for server
struct JmyServerConfig {
#if USE_CONNECTOR_AND_SESSION
	JmySessionConfig session_conf;
#else
	JmyConnectionConfig conn_conf;
#endif
	char* listen_ip;
	unsigned short listen_port;
	unsigned int max_conn;
};

struct JmyTotalReconnInfo {
	unsigned short send_count, recv_count;
};
