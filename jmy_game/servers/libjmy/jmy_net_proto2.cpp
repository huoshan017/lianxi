#include "jmy_net_proto2.h"
#include "jmy_datatype.h"
#include "jmy_log.h"

#define PACK_INT16_TO_BUFF(id, buff) ({ \
			buff[0] = (id>>8) & 0xff; \
			buff[1] = id & 0xff; \
			2; \
		})

#define UNPACK_INT16_FROM_BUFF(buff) ({ \
			((buff[0]<<8)&0xff00) + (buff[1]&0xff); \
		})

#define PACK_INT32_TO_BUFF(id, buff) ({ \
			buff[0] = (id>>24) & 0xff; \
			buff[1] = (id>>16) & 0xff; \
			buff[2] = (id>>8) & 0xff; \
			buff[3] = id & 0xff; \
			4; \
		})

#define UNPACK_INT32_FROM_BUFF(buff) ({ \
			((buff[0]<<24)&0xff000000) + \
			((buff[1]<<16)&0xff0000) + \
			((buff[2]<<8)&0xff00) + \
			(buff[3]&0xff); \
		})

#define PACK_INT64_TO_BUFF(value, buff) ({ \
			buf[0] = (value>>56) & 0xff; \
			buf[1] = (value>>48) & 0xff; \
			buf[2] = (value>>40) & 0xff; \
			buf[3] = (value>>32) & 0xff; \
			buf[4] = (value>>24) & 0xff; \
			buf[5] = (value>>16) & 0xff; \
			buf[6] = (value>>8) & 0xff; \
			buf[7] = (value) & 0xff; \
			8; \
		})

#define UNPACK_INT64_FROM_BUFF(buff) ({ \
			((buff[0]<<56)&0xff00000000000000) + \
			((buff[1]<<48)&0xff000000000000) + \
			((buff[2]<<40)&0xff0000000000) + \
			((buff[3]<<32)&0xff00000000) + \
			((buff[4]<<24)&0xff000000) + \
			((buff[5]<<16)&0xff0000) + \
			((buff[6]<<8)&0xff00) + \
			(buff[7]&0xff); \
		})


int jmy_net_proto2_pack_disconnect(char* buf, unsigned char len) {
	int pack_len = jmy_net_proto2_disconnect_pack_len();
	if (!buf || len < pack_len)
		return -1;

	// head
	PACK_INT16_TO_BUFF(JMY_PACKET_LEN_TYPE, buf);
	// type
	buf[2] = (char)JMY_PACKET_DISCONNECT;
	return pack_len;
}

int jmy_net_proto2_pack_disconnect_ack(char* buf, unsigned char len) {
	int pack_len = jmy_net_proto2_disconnect_ack_pack_len();
	if (!buf || len < pack_len)
		return -1;

	// head
	PACK_INT16_TO_BUFF(JMY_PACKET_LEN_TYPE, buf);
	// type
	buf[2] = (char)JMY_PACKET_DISCONNECT_ACK;
	return pack_len;
}

int jmy_net_proto2_pack_user_data_head(char* buf, unsigned char len, int msg_id, unsigned short data_len) {
	int pack_len = jmy_net_proto2_user_data_pack_len(data_len);
	if (!buf || len < pack_len)
		return -1;

	// head
	PACK_INT16_TO_BUFF((pack_len-JMY_PACKET_LEN_HEAD), buf);
	// type
	buf[2] = (char)JMY_PACKET_USER_DATA;
	// msg_id
	PACK_INT16_TO_BUFF((msg_id), (buf+3));
	return jmy_net_proto2_user_data_pack_len();
}

int jmy_net_proto2_pack_user_id_data_head(char* buf, unsigned char len, int user_id, int msg_id, unsigned short data_len)
{
	int pack_len = jmy_net_proto2_user_id_data_pack_len(data_len);
	if (!buf || len < pack_len)
		return -1;

	// head
	PACK_INT16_TO_BUFF((pack_len-JMY_PACKET_LEN_HEAD), buf);
	// type
	buf[2] = (char)JMY_PACKET_USER_ID_DATA;
	int offset = 3;
	// user_id
	int res = PACK_INT32_TO_BUFF(user_id, (buf+offset));
	offset += res;
	// msg_id
	res = PACK_INT16_TO_BUFF((msg_id), (buf+offset));
	return jmy_net_proto2_user_id_data_pack_len();
}

#if 0
int jmy_net_proto2_pack_ack(char* buf, unsigned char len, unsigned short msg_count, unsigned short curr_id) {
	int pack_len = jmy_net_proto2_ack_pack_len();
	if (!buf || len < pack_len)
		return -1;

	PACK_INT16_TO_BUFF((pack_len-JMY_PACKET_LEN_HEAD), buf);
	buf[2] = (char)JMY_PACKET_ACK;
	PACK_INT16_TO_BUFF(msg_count, (buf+3));
	PACK_INT16_TO_BUFF(curr_id, (buf+5));
	return pack_len;
}
#endif

int jmy_net_proto2_pack_heartbeat(char* buf, unsigned char len) {
	int pack_len = jmy_net_proto2_heartbeat_pack_len();
	if (!buf || len < pack_len)
		return -1;

	// head
	PACK_INT16_TO_BUFF((pack_len-JMY_PACKET_LEN_HEAD), buf);
	// type
	buf[2] = (char)JMY_PACKET_HEARTBEAT;
	// timestamp
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	std::time_t t = std::chrono::system_clock::to_time_t(now);
	PACK_INT32_TO_BUFF(t, (buf+3));
	return pack_len;
}

int jmy_net_proto_unpack_data(JmyPacketType packet_type, const char* buf, unsigned short len, JmyPacketUnpackData& data, int conn_id, void* param) {
	if (!buf || !len)
		return 0;

	int handled = 0;
	switch (packet_type) {
	case JMY_PACKET_USER_DATA:
	case JMY_PACKET_USER_ID_DATA:
		{
			int offset = JMY_PACKET_LEN_TYPE;
			int user_id = 0;
			// get user_id
			if (data.type == JMY_PACKET_USER_ID_DATA) {
				if (len - offset < 4) {
					data.result = JMY_UNPACK_RESULT_DATA_NOT_ENOUGH;
					LibJmyLogDebug("user id %d length not enough", len-offset);
					return 0;
				}
				user_id = UNPACK_INT32_FROM_BUFF((buf+offset));
				offset += 4;
			}
			// msg id
			data.param = (void*)(long)UNPACK_INT16_FROM_BUFF((buf+offset));
			offset += 2;
			// msg info
			JmyMsgInfo* msg_info = (JmyMsgInfo*)&data.msg_info;
			msg_info->user_id = user_id;
			msg_info->msg_id = (int)(long)data.param;
			msg_info->data = const_cast<char*>(buf+offset);
			msg_info->len = len-offset;
			msg_info->conn_id = conn_id;
			msg_info->param = param;
			handled = offset+msg_info->len;
		}
		break;
	case JMY_PACKET_HEARTBEAT:
		{
			if (len < JMY_PACKET_LEN_TIMESTAMP) {
				data.result = JMY_UNPACK_RESULT_DATA_NOT_ENOUGH;
				return 0;
			}
			// time
			data.param = (void*)(long)(UNPACK_INT32_FROM_BUFF(buf));
			handled = JMY_PACKET_LEN_TIMESTAMP;
		}
		break;
	case JMY_PACKET_DISCONNECT:
	case JMY_PACKET_DISCONNECT_ACK:
		{
			handled = 0;
		}
		break;
	default:
		LibJmyLogWarn("handle packet type(%d) invalid", (int)data.type);
		return -1;
	}
	data.result = JMY_UNPACK_RESULT_NO_ERROR;
	return handled;
}
