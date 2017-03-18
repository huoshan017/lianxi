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
#if USE_CONN_PROTO
	JMY_PACKET_CONNECT			= 1,
	JMY_PACKET_CONNECT_RESULT	= 2,
	JMY_PACKET_RECONNECT		= 3,
	JMY_PACKET_RECONNECT_RESULT	= 4,
#endif
	JMY_PACKET_USER_DATA		= 5,
	JMY_PACKET_ACK				= 6,
	JMY_PACKET_HEARTBEAT		= 7,
	JMY_PACKET_DISCONNECT		= 8,
	JMY_PACKET_DISCONNECT_ACK	= 9,
};

#if USE_CONN_PROTO
enum { JMY_CONN_LEN_RES_ID		= 4, };
enum { JMY_CONN_LEN_RES_SESSION = 16, };
#endif

enum JmyUnpackResultType {
	JMY_UNPACK_RESULT_NO_ERROR				= 0,
	JMY_UNPACK_RESULT_DATA_NOT_ENOUGH		= 1,
	JMY_UNPACK_RESULT_USER_DATA_NOT_ENOUGH	= 2,
	JMY_UNPACK_RESULT_MSG_LEN_INVALID		= 3,
};

#if USE_CONN_PROTO
enum { JMY_PACKET_LEN_CONN				= 1, };
enum { JMY_PACKET_LEN_CONN_RES			= 1 + JMY_CONN_LEN_RES_ID + JMY_CONN_LEN_RES_SESSION, };
enum { JMY_PACKET_LEN_RECONN			= 1 + JMY_CONN_LEN_RES_ID + JMY_CONN_LEN_RES_SESSION, };
enum { JMY_PACKET_LEN_RECONN_RES		= 1 + JMY_CONN_LEN_RES_ID + JMY_CONN_LEN_RES_SESSION, };
#endif
enum { JMY_PACKET_LEN_DISCONNECT		= 1, };
enum { JMY_PACKET_LEN_DISCONNECT_ACK	= 1, };
enum { JMY_PACKET_LEN_USER_DATA_HEAD	= 1+2+2, };
enum { JMY_PACKET_LEN_ACK				= 1+2+2, };
enum { JMY_PACKET_LEN_HEARTBEAT			= 1+4, };

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

#if USE_CONN_PROTO
/**
 * pack connect
 */
int jmy_net_proto_pack_connect(char* buf, unsigned char len);

/**
 * pack connect result
 */
int jmy_net_proto_pack_connect_result(char* buf, unsigned char len, unsigned int id, char session[JMY_CONN_LEN_RES_SESSION]);

/**
 * pack reconnect
 */
int jmy_net_proto_pack_reconnect(char* buf, unsigned char len, unsigned int id, char session[JMY_CONN_LEN_RES_SESSION]);

/**
 * pack reconnect result
 */
int jmy_net_proto_pack_reconnect_result(char* buf, unsigned char len, unsigned int id, char session[JMY_CONN_LEN_RES_SESSION]);
#endif

/**
 * pack disconnect
 */
int jmy_net_proto_pack_disconnect(char* buf, unsigned char len);

/**
 * pack disconnect ack
 */
int jmy_net_proto_pack_disconnect_ack(char* buf, unsigned char len);

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

#if USE_CONN_PROTO
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
#endif
