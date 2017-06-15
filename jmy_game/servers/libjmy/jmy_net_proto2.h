#pragma once

#include <chrono>
#include <string>
#include <unordered_map>
#include "jmy_datatype.h"
#include "jmy_const.h"
#include "jmy_singleton.hpp"
#include "jmy_log.h"

#define USE_CONN_PROTO 0

enum JmyPacketType {
	JMY_PACKET_NONE				= 0,
	JMY_PACKET_USER_DATA		= 5,
	JMY_PACKET_ACK				= 6,
	JMY_PACKET_HEARTBEAT		= 7,
	JMY_PACKET_DISCONNECT		= 8,
	JMY_PACKET_DISCONNECT_ACK	= 9,
	JMY_PACKET_USER_ID_DATA		= 10,
	// user defined type 100-254
	JMY_PACKET_TYPE_USER		= 100,
	JMY_PACKET_TYPE_MAX 		= 255
};

enum JmyUnpackResultType {
	JMY_UNPACK_RESULT_NO_ERROR				= 0,
	JMY_UNPACK_RESULT_DATA_NOT_ENOUGH		= 1,
	JMY_UNPACK_RESULT_USER_DATA_NOT_ENOUGH	= 2,
	JMY_UNPACK_RESULT_MSG_LEN_INVALID		= 3,
};

enum { JMY_PACKET_LEN_HEAD				= 2, };
enum { JMY_PACKET_LEN_TYPE 				= 1, };
enum { JMY_PACKET_LEN_USER_ID			= 4, };
enum { JMY_PACKET_LEN_MSG_ID			= 2, };
enum { JMY_PACKET_LEN_TIMESTAMP			= 4, };

// active close default timeout
enum { JMY_ACTIVE_CLOSE_CONNECTION_TIMEOUT = 30, };

struct JmyPacketUnpackData {
	JmyPacketType type;
	void* param;
	int data;
	JmyUnpackResultType result;
	JmyMsgInfo msg_info;
	JmyPacketUnpackData() : type(JMY_PACKET_NONE), param(nullptr), data(0), result(JMY_UNPACK_RESULT_NO_ERROR) {
	}
};

int jmy_net_proto2_disconnect_pack_len() {
	return JMY_PACKET_LEN_HEAD + JMY_PACKET_LEN_TYPE;
}

int jmy_net_proto2_disconnect_ack_pack_len() {
	return JMY_PACKET_LEN_HEAD + JMY_PACKET_LEN_TYPE;
}

int jmy_net_proto2_user_data_pack_len() {
	return JMY_PACKET_LEN_HEAD + JMY_PACKET_LEN_TYPE + JMY_PACKET_LEN_MSG_ID;
}

int jmy_net_proto2_user_data_pack_len(unsigned short data_len) {
	return  jmy_net_proto2_user_data_pack_len() + data_len;
}

int jmy_net_proto2_user_id_data_pack_len() {
	return JMY_PACKET_LEN_HEAD + JMY_PACKET_LEN_TYPE + JMY_PACKET_LEN_USER_ID + JMY_PACKET_LEN_MSG_ID;
}

int jmy_net_proto2_user_id_data_pack_len(unsigned short data_len) {
	return jmy_net_proto2_user_id_data_pack_len() + data_len;
}

#if 0
int jmy_net_proto2_ack_pack_len() {
	return JMY_PACKET_LEN_HEAD + JMY_PACKET_LEN_TYPE;
}
#endif

int jmy_net_proto2_heartbeat_pack_len() {
	return JMY_PACKET_LEN_HEAD + JMY_PACKET_LEN_TYPE + JMY_PACKET_LEN_TIMESTAMP;
}

/**
 * pack disconnect
 */
int jmy_net_proto2_pack_disconnect(char* buf, unsigned char len);

/**
 * pack disconnect ack
 */
int jmy_net_proto2_pack_disconnect_ack(char* buf, unsigned char len);

/**
 * pack msg id and user data
 */
int jmy_net_proto2_pack_user_data_head(char* buf, unsigned char len, int msgid, unsigned short data_len);

/**
 * pack user id, msg id, user data
 */
int jmy_net_proto2_pack_user_id_data_head(char* buf, unsigned char len, int user_id, int msg_id, unsigned short data_len);

#if 0
/**
 * pack ack
 */
int jmy_net_proto2_pack_ack(char* buf, unsigned char len, unsigned short msg_count, unsigned short curr_id);
#endif

/**
 * pack heartbeat
 */
int jmy_net_proto2_pack_heartbeat(char* buf, unsigned char len);

/**
 * unpack data
 */
int jmy_net_proto2_unpack_data(const char* buf, unsigned short len, JmyPacketUnpackData& data, int conn_id, void* param);
