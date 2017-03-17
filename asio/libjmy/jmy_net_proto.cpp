#include "jmy_net_proto.h"
#include "jmy_datatype.h"
#include "jmy_mem.h"
#include "jmy_log.h"
#include <random>

#define PACK_INT16_TO_BUFF(id, buff) ({ \
			buff[0] = (id>>8) & 0xff; \
			buff[1] = id & 0xff; \
			2; \
		})

#define PACK_INT32_TO_BUFF(id, buff) ({ \
			buff[0] = (id>>24) & 0xff; \
			buff[1] = (id>>16) & 0xff; \
			buff[2] = (id>>8) & 0xff; \
			buff[3] = id & 0xff; \
			4; \
		})

#if USE_CONN_PROTO
int jmy_net_proto_pack_connect(char* buf, unsigned char len/*, JmyConnType connect_type*/) {
	if (!buf || len < PacketConnLen) return -1;
	buf[0] = (char)JmyPacketConnect;
	return PacketConnLen;
}

int jmy_net_proto_pack_connect_result(char* buf, unsigned char len, /*JmyConnType connect_type, */unsigned int id, char session[ConnResSessionLen]) {
	if (!buf || len < PacketConnResLen) return -1;
	buf[0] = (char)JmyPacketConnectResult;
	unsigned char offset = PACK_INT32_TO_BUFF(id, (buf+1));
	std::memcpy(buf+1+offset, session, ConnResSessionLen);
	return PacketConnResLen;
}

int jmy_net_proto_pack_reconnect(char* buf, unsigned char len, unsigned int id, char session[ConnResSessionLen]) {
	if (!buf || len < PacketReconnLen) return -1;
	buf[0] = (char)JmyPacketReconnect;
	unsigned char offset = PACK_INT32_TO_BUFF(id, (buf+1));
	std::memcpy(buf+1+offset, session, ConnResSessionLen);
	return PacketReconnLen;
}

int jmy_net_proto_pack_reconnect_result(char* buf, unsigned char len, unsigned int id, char session[ConnResSessionLen]) {
	if (!buf || len < PacketReconnResLen) return -1;
	buf[0] = (char)JmyPacketReconnectResult;
	unsigned char offset = PACK_INT32_TO_BUFF(id, (buf+1));
	std::memcpy(buf+1+offset, session, ConnResSessionLen);
	return PacketReconnResLen;
}
#endif

int jmy_net_proto_pack_msgid(char* buf, unsigned char len, int msgid, unsigned short data_len) {
	if (!buf || len < PacketUserDataHeadLen) return -1;
	buf[0] = (char)JmyPacketUserData;
	buf[1] = ((data_len+2)>>8) & 0xff;
	buf[2] = (data_len+2) & 0xff;
	buf[3] = (msgid>>8) & 0xff;
	buf[4] = msgid & 0xff;
	return PacketUserDataHeadLen;
}

int jmy_net_proto_pack_ack(char* buf, unsigned char len, unsigned short msg_count, unsigned short curr_id) {
	if (!buf || len < PacketAckLen) return -1;
	buf[0] = (char)JmyPacketAck;
	buf[1] = (msg_count>>8) & 0xff;
	buf[2] = msg_count & 0xff;
	buf[3] = (curr_id>>8) & 0xff;
	buf[4] = curr_id & 0xff;
	return PacketAckLen;
}

int jmy_net_proto_pack_heartbeat(char* buf, unsigned char len) {
	if (!buf || len < PacketHeartbeatLen) return -1;
	buf[0] = (char)JmyPacketHeartbeat;
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	std::time_t t = std::chrono::system_clock::to_time_t(now);
	buf[1] = (t>>24) & 0xff;
	buf[2] = (t>>16) & 0xff;
	buf[3] = (t>>8) & 0xff;
	buf[4] = t & 0xff;
	return PacketHeartbeatLen;
}

int jmy_net_proto_unpack_data_head(const char* buf, unsigned int len, JmyPacketUnpackData& data, int session_id, void* param) {
	if (!buf || !len)
		return 0;

	int handled = 0;
	data.type = (JmyPacketType)buf[0];
	switch (data.type) {
#if USE_CONN_PROTO
	case JmyPacketConnect:
		{
			if (len < PacketConnLen) return 0;
			handled = PacketConnLen;
		}
		break;
	case JmyPacketConnectResult:
		{
			if (len < PacketConnResLen) return 0;
			// id
			data.data = (int)((buf[1]<<8)&0xff00 + buf[2]&0xff);
			// session
			char* session = (char*)jmy_mem_malloc(ConnResSessionLen);
			std::memcpy(session, (void*)(buf+2), ConnResSessionLen);
			handled = PacketConnResLen;
		}
		break;
	case JmyPacketReconnect:
		{
			if (len < PacketReconnLen) return 0;
			// id
			data.data = (int)((buf[1]<<8)&0xff00 + buf[2]&0xff);
			// session
			char* session = (char*)jmy_mem_malloc(ConnResSessionLen);
			std::memcpy(session, (void*)(buf+2), ConnResSessionLen);
			handled = PacketReconnLen;
		}
		break;
	case JmyPacketReconnectResult:
		{
			if (len < PacketReconnResLen) return 0;
			// id
			data.data = (int)((buf[1]<<8)&0xff00 + buf[2]&0xff);
			// session
			char* session = (char*)jmy_mem_malloc(ConnResSessionLen);
			std::memcpy(session, (void*)(buf+2), ConnResSessionLen);
			handled = PacketReconnResLen;
		}
		break;
#endif
	case JmyPacketUserData:
		{
			// head not enough
			if ((int)len-1-2 < 0) {
				data.result = JmyPacketUnpackDataNotEnough;
				LibJmyLogInfo("data length %d not enough", len);
				return 0;
			}
			// data len invalid, at least include msg id (2 bytes)
			unsigned short data_len = ((buf[1]<<8)&0xff00) + (buf[2]&0xff);
			if (data_len < 2) {
				data.data = data_len;
				data.result = JmyPacketUnpackMsgLenInvalid;
				LibJmyLogInfo("message data len field length(%d) invalid", data_len);
				return -1;
			}
			// (can write_len + can read len - nhandled) is not enough to hold next messag	
			if ((int)len-1-2 < (int)data_len) {
				data.data = data_len+1+2; // next message len(include data head)
				data.result = JmyPacketUnpackUserDataNotEnough;
				//LibJmyLogInfo("next message len %d not enough, need %d", len, data_len);
				return 0;
			}
			// msg id
			data.param = (void*)(long)(((buf[3]<<8)&0xff00) + (buf[4]&0xff));
			JmyMsgInfo* msg_info = (JmyMsgInfo*)&data.msg_info;
			msg_info->msg_id = (int)(long)data.param;
			msg_info->data = const_cast<char*>(buf+1+2+2);
			msg_info->len = data_len-2;
			msg_info->session_id = session_id;
			msg_info->param = param;
			handled = 1+2+2+msg_info->len;
		}
		break;
	case JmyPacketAck:
		{
			if (len-1 <= 2+2) {
				data.result = JmyPacketUnpackDataNotEnough;
				return 0;
			}
			// ack count
			data.data = (int)((buf[1]<<8)&0xff00 + buf[2]&0xff);
			// curr id
			data.param = (void*)(long)((buf[3]<<8)&0xff00 + buf[4]&0xff);	
			handled = 1+2+2;
		}
		break;
	case JmyPacketHeartbeat:
		{
			if (len-1 <= 4) {
				data.result = JmyPacketUnpackDataNotEnough;
				return 0;
			}
			// time
			data.param = (void*)(long)((buf[1]<<24)&0xff000000 + (buf[2]<<16)&0xff0000 + (buf[3]<<8)&0xff00 + buf[4]&0xff);
			handled = 1+4;
		}
		break;
	default:
		LibJmyLogWarn("handle packet type(%d) invalid", (int)data.type);
		return -1;
	}
	data.result = JmyPacketUnpackNoError;
	return handled;
}

bool jmy_id_to_session_info(int session_id, JmySessionInfo& info)
{
	info.type = (JmyConnType)((session_id>>24) & 0xff);
	info.session_id = session_id & 0xffffff;
	return true;
}

int jmy_session_info_to_id(const JmySessionInfo& info)
{
	return (int)((info.type<<24)&0xff000000 + (info.session_id&0x00ffffff));
}

unsigned short jmy_ack_id_add(unsigned short curr_id, unsigned short increment)
{
	if (curr_id + increment > JMY_ACK_END_ID) {
		curr_id = curr_id + increment - JMY_ACK_END_ID + JMY_ACK_START_ID;
	} else {
		curr_id += increment;
	}
	return curr_id;
}

#if USE_CONN_PROTO
// JmyConnIdAndSessionMgr
JmyConnIdAndSessionMgr::JmyConnIdAndSessionMgr() : curr_id_(0)
{
}

JmyConnIdAndSessionMgr::~JmyConnIdAndSessionMgr()
{
	clear();
}

void JmyConnIdAndSessionMgr::clear()
{
	id2sessions_.clear();
}
	
bool JmyConnIdAndSessionMgr::newIdAndSession(unsigned int& id, char*& session)
{
	static char cs[] = "abcdefghijklmnopqrstuvwxyz0123456789~!@#$%^&*()_+`-={}[]:<>?,./";
	curr_id_ += 1;
	if (curr_id_ > MaxId) {
		curr_id_ = 1;
	}
	// generate session string
	std::default_random_engine gen;
	std::uniform_int_distribution<> dis(0, sizeof(cs));
	session = (char*)jmy_mem_malloc(ConnResSessionLen);
	for (int i=0; i<ConnResSessionLen; ++i) {
		session[i] = dis(gen);
	}
	id = curr_id_;
	id2sessions_.insert(std::make_pair(id, session));
	return true;
}

bool JmyConnIdAndSessionMgr::removeById(unsigned int id)
{
	std::unordered_map<unsigned int, char*>::iterator it = id2sessions_.find(id);
	if (it == id2sessions_.end())
		return false;
	if (it->second)
		jmy_mem_free(it->second);
	id2sessions_.erase(id);
	return true;
}
#endif
