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
	JMY_PACKET_USER_ID_DATA		= 10,
	// user defined type 100-254
	JMY_PACKET_TYPE_USER		= 100,
	JMY_PACKET_TYPE_MAX 		= 255
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

enum { JMY_PACKET_LEN_TYPE 				= 1, };
enum { JMY_PACKET_LEN_DISCONNECT		= 1, };
enum { JMY_PACKET_LEN_DISCONNECT_ACK	= 1, };
enum { JMY_PACKET_LEN_USER_DATA_HEAD	= 1+2+2, };
enum { JMY_PACKET_LEN_USER_ID_DATA_HEAD = 1+4+2+2, };
enum { JMY_PACKET_LEN_ACK				= 1+2+2, };
enum { JMY_PACKET_LEN_HEARTBEAT			= 1+4, };

struct JmyPacketUnpackData {
	JmyPacketType type;
	void* param;
	int data;
	JmyUnpackResultType result;
	JmyMsgInfo msg_info;
	JmyPacketUnpackData() : type(JMY_PACKET_NONE), param(nullptr), data(0), result(JMY_UNPACK_RESULT_NO_ERROR) {
	}
};

/**
 * get pack type
 */
JmyPacketType jmy_net_proto_pack_type(const char* buf);

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
 * pack msg id and user data length
 */
int jmy_net_proto_pack_msgid_datalen(char* buf, unsigned char len, int msgid, unsigned short data_len);

/**
 * pack user id, msg id, user data length
 */
int jmy_net_proto_pack_userid_msgid_datalen(char* buf, unsigned char len, int user_id, int msg_id, unsigned short data_len);

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
 * defined type
 */
template <typename T>
int jmy_net_proto_pack_defined_type_head(char* buf, unsigned short len, unsigned char defined_pack_type, const T& defined_pack_param);

/**
 * int64 type defined
 */
template <>
int jmy_net_proto_pack_defined_type_head<int64_t>(char* buf, unsigned short len, unsigned char defined_pack_type, const int64_t& int_pack_param);

/**
 * int type defined
 */
template <>
int jmy_net_proto_pack_defined_type_head<int>(char* buf, unsigned short len, unsigned char defined_pack_type, const int& int_pack_param);

/**
 * short type defined
 */
template <>
int jmy_net_proto_pack_defined_type_head<short>(char* buf, unsigned short len, unsigned char defined_pack_type, const short& short_pack_param);

/**
 * char type defined
 */
template <>
int jmy_net_proto_pack_defined_type_head<char>(char* buf, unsigned short len, unsigned char defined_pack_type, const char& char_pack_param);

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
