#pragma once

#include <chrono>
#include "jmy_datatype.h"
#include "jmy_log.h"

enum JmyPacketType {
	JmyPacketUserData	= 0,
	JmyPacketAck		= 1,
	JmyPacketHeartbeat	= 2,
};

enum JmyPacketUnpackResult {
	JmyPacketUnpackNoError				= 0,
	JmyPacketUnpackDataNotEnough		= 1,
	JmyPacketUnpackUserDataNotEnough	= 2,
	JmyPacketUnpackMsgLenInvalid		= 3,
};

enum { UserDataHeadLen = 5, };
enum { AckHeadLen = 3, };
enum { HeartbeatHeadLen = 5, };

struct JmyPacketUnpackData {
	JmyPacketType type;
	void* param;
	int data;
	JmyPacketUnpackResult result;
	JmyMsgInfo msg_info;
	JmyPacketUnpackData() : type(JmyPacketUserData), param(NULL), data(0), result(JmyPacketUnpackNoError) {
	}
};

/**
 * pack use data
 */
int jmy_net_proto_pack_msgid(char* buf, unsigned char len, int msgid, unsigned short data_len);

/**
 * pack ack
 */
int jmy_net_proto_pack_ack(char* buf, unsigned char len, unsigned short msg_count);

/**
 * pack heartbeat
 */
int jmy_net_proto_pack_heartbeat(char* buf, unsigned char len);

/**
 * unpack data
 */
int jmy_net_proto_unpack_data_head(const char* buf, unsigned int len, JmyPacketUnpackData& data, int session_id, void* param);
