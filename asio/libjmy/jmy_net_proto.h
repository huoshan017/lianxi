#pragma once

#include <chrono>
#include <string>
#include <unordered_map>
#include "jmy_datatype.h"
#include "jmy_const.h"
#include "jmy_singleton.hpp"
#include "jmy_log.h"

enum JmyPacketType {
	JmyPacketConnect		= 0,
	JmyPacketAckConnect		= 1,
	JmyPacketReconnect		= 2,
	JmyPacketAckReconnect	= 3,
	JmyPacketUserData		= 4,
	JmyPacketAck			= 5,
	JmyPacketHeartbeat		= 6,
};

enum JmyConnType {
	JmyConnTypeNormal = 0,
	JmyConnTypeReconn = 1,
};

enum { AckReconnIdLen = 4, };
enum { AckReconnSessionLen = 16, };

enum JmyPacketUnpackResult {
	JmyPacketUnpackNoError				= 0,
	JmyPacketUnpackDataNotEnough		= 1,
	JmyPacketUnpackUserDataNotEnough	= 2,
	JmyPacketUnpackMsgLenInvalid		= 3,
};

enum { PacketConnLen = 1, };
enum { PacketAckConnLen = 1+AckReconnIdLen+AckReconnSessionLen, };
enum { PacketReconnLen = 1+AckReconnIdLen+AckReconnSessionLen, };
enum { PacketAckReconnLen = 1+AckReconnIdLen+AckReconnSessionLen, };
enum { PacketUserDataHeadLen = 1+2+2, };
enum { PacketAckLen = 1+2+2, };
enum { PacketHeartbeatLen = 1+4, };

struct JmyPacketUnpackData {
	JmyPacketType type;
	void* param;
	int data;
	JmyPacketUnpackResult result;
	JmyMsgInfo msg_info;
	JmyPacketUnpackData() : type(JmyPacketUserData), param(nullptr), data(0), result(JmyPacketUnpackNoError) {
	}
};


/**
 * pack connect
 */
int jmy_net_proto_pack_connect(char* buf, unsigned char len/*, unsigned char connect_type*/);

/**
 * pack ack connect
 */
int jmy_net_proto_pack_ack_connect(char* buf, unsigned char len, unsigned int id, char session[AckReconnSessionLen]);

/**
 * pack reconnect
 */
int jmy_net_proto_pack_reconnect(char* buf, unsigned char len, unsigned int id, char session[AckReconnSessionLen]);

/**
 * pack ack reconnect
 */
int jmy_net_proto_pack_ack_reconnect(char* buf, unsigned char len, unsigned int id, char session[AckReconnSessionLen]);

/**
 * pack use data
 */
int jmy_net_proto_pack_msgid(char* buf, unsigned char len, int msgid, unsigned short data_len);

/**
 * pack ack
 */
int jmy_net_proto_pack_ack(char* buf, unsigned char len, unsigned short msg_count, unsigned short curr_id);

/**
 * pack heartbeat
 */
int jmy_net_proto_pack_heartbeat(char* buf, unsigned char len);

/**
 * unpack data
 */
int jmy_net_proto_unpack_data_head(const char* buf, unsigned int len, JmyPacketUnpackData& data, int session_id, void* param);

/**
 * sessoin info 
 */
bool jmy_id_to_session_info(int session_id, JmySessionInfo& info);
int jmy_session_info_to_id(const JmySessionInfo& info);

/**
 * ack id
 */
unsigned short jmy_ack_id_add(unsigned short curr_id, unsigned short increment);
unsigned short jmy_ack_id_diff(unsigned short self_id, unsigned short ack_id);

/**
 * connection id and session manager
 */
class JmyConnIdAndSessionMgr : public JmySingleton<JmyConnIdAndSessionMgr>
{
public:
	JmyConnIdAndSessionMgr();
	~JmyConnIdAndSessionMgr();
	
	void clear();
	bool newIdAndSession(unsigned int&, char*&);
	bool removeById(unsigned int);

private:
	enum { MaxId = 10000000, };
	unsigned int curr_id_;
	std::unordered_map<unsigned int, char*> id2sessions_;
};

#define ConnIdSessionMgr (JmyConnIdAndSessionMgr::getInstance())
