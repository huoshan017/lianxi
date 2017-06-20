using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace network_test
{
    public class net_proto
    {
        public enum PacketType {
            PACKET_NONE = 0,
            PACKET_USER_DATA = 5,
            PACKET_ACK = 6,
            PACKET_HEARTBEAT = 7,
            PACKET_DISCONNECT = 8,
            PACKET_DISCONNECT_ACK = 9,
            //PACKET_USER_ID_DATA = 10,
        };

        public enum UnpackResultType {
            UNPACK_RESULT_NO_ERROR = 0,
            UNPACK_RESULT_DATA_NOT_ENOUGH = 1,
            UNPACK_RESULT_USER_DATA_NOT_ENOUGH = 2,
            UNPACK_RESULT_MSG_LEN_INVALID = 3,
        };

        public const int PACKET_LEN_HEAD = 2;
        public const int PACKET_LEN_TYPE = 1;
        public const int PACKET_LEN_MSG_ID = 2;
        public const int PACKET_LEN_TIMESTAMP = 4;

        public struct MsgInfo {
            public int msg_id;
            public byte[] buf;
            public ushort offset;
            public ushort len;
        };

        public struct PacketUnpackData {
            public PacketType type;
            public int param;
            public int data;
            public UnpackResultType result_type;
            public MsgInfo msg_info;
        };

        private static void pack_int16_to_buff(Int16 value, ref byte[] byte_data, int offset)
        {
            byte_data[offset+0] = (byte)((value >> 8) & 0xff);
            byte_data[offset+1] = (byte)(value & 0xff);
        }

        private static Int16 unpack_int16_from_buff(byte[] byte_data, int offset)
        {
            return (Int16)(((byte_data[offset+0] << 8) & 0xff00) + (byte_data[offset+1] & 0xff));
        }

        private static void pack_int32_to_buff(Int32 value, ref byte[] byte_data, int offset)
        {
            byte_data[offset+0] = (byte)((value>>24)&0xff);
            byte_data[offset+1] = (byte)((value>>16)&0xff);
            byte_data[offset+2] = (byte)((value>>8)&0xff);
            byte_data[offset+3] = (byte)(value&0xff);
        }

        private static Int32 unpack_int32_from_buff(byte[] byte_data, int offset)
        {
            return (Int32)(((byte_data[offset+0]<<24)&0xff000000) + ((byte_data[offset+1]<<16)&0xff0000) + ((byte_data[offset+2]<<8)&0xff00) + (byte_data[offset+3]&0xff));
        }

        public static bool net_proto_get_packet_len_type(byte[] byte_data, ref PacketType type, ref int len)
        {
            if (byte_data.Length < PACKET_LEN_HEAD + PACKET_LEN_TYPE)
            {
                return false;
            }

            len = unpack_int16_from_buff(byte_data, 0);
            type = (PacketType)byte_data[2];
            return true;
        }

        public static int net_proto_disconnect_pack_len()
        {
            return PACKET_LEN_HEAD + PACKET_LEN_TYPE;
        }

        public static int net_proto_disconnect_ack_pack_len()
        {
            return PACKET_LEN_HEAD + PACKET_LEN_TYPE;
        }

        public static int net_proto_user_data_pack_len()
        {
            return PACKET_LEN_HEAD + PACKET_LEN_TYPE + PACKET_LEN_MSG_ID;
        }

        public static int net_proto_user_data_pack_len(ushort data_len)
        {
            return net_proto_user_data_pack_len() + data_len;
        }

        /*public static int net_proto_user_id_data_pack_len()
        {
            return PACKET_LEN_HEAD + PACKET_LEN_TYPE + PACKET_LEN_USER_ID + PACKET_LEN_MSG_ID;
        }

        public static int net_proto_user_id_data_pack_len(ushort data_len)
        {
            return net_proto_user_id_data_pack_len() + data_len;
        }*/

        public static int net_proto_heartbeat_pack_len()
        {
            return PACKET_LEN_HEAD + PACKET_LEN_TYPE + PACKET_LEN_TIMESTAMP;
        }

        public static int net_proto_pack_disconnect(ref byte[] byte_data)
        {
            int pack_len = net_proto_disconnect_pack_len();
            if (pack_len > byte_data.Length)
                return -1;
            pack_int16_to_buff((short)PACKET_LEN_TYPE, ref byte_data, 0);
            byte_data[2] = (byte)PacketType.PACKET_DISCONNECT;
            return pack_len;
        }

        public static int net_proto_pack_disconnect_ack(ref byte[] byte_data)
        {
            int pack_len = net_proto_disconnect_ack_pack_len();
            if (pack_len > byte_data.Length)
                return -1;
            pack_int16_to_buff((short)PACKET_LEN_TYPE, ref byte_data, 0);
            byte_data[2] = (byte)PacketType.PACKET_DISCONNECT_ACK;
            return pack_len;
        }

        public static int net_proto_pack_user_data_head(ref byte[] byte_data, int msg_id, ushort data_len)
        {
            int pack_len = net_proto_user_data_pack_len(data_len);
            if (pack_len-data_len > byte_data.Length)
                return -1;

            // head
            pack_int16_to_buff((short)(pack_len-PACKET_LEN_HEAD), ref byte_data, 0);
            // type
            byte_data[2] = (byte)PacketType.PACKET_USER_DATA;
            // msg id
            pack_int16_to_buff((short)msg_id, ref byte_data, 3);
            return net_proto_user_data_pack_len();
        }

        /*public static int net_proto_pack_user_id_data_head(ref byte[] byte_data, int user_id, int msg_id, ushort data_len)
        {
            int pack_len = net_proto_user_id_data_pack_len(data_len);
            if (pack_len - data_len > byte_data.Length)
                return -1;

            // head
            pack_int16_to_buff((short)(pack_len-PACKET_LEN_HEAD), ref byte_data, 0);
            // type
            byte_data[2] = (byte)PacketType.PACKET_USER_ID_DATA;
            // user id
            pack_int32_to_buff(user_id, ref byte_data, 3);
            pack_int16_to_buff((short)msg_id, ref byte_data, 7);
            return net_proto_user_id_data_pack_len();
        }*/

        public static int net_proto_pack_heartbeat(ref byte[] byte_data)
        {
            int pack_len = net_proto_heartbeat_pack_len();
            if (pack_len > byte_data.Length)
                return -1;
            // head
            pack_int16_to_buff((short)(pack_len-PACKET_LEN_HEAD), ref byte_data, 0);
            // type
            byte_data[2] = (byte)PacketType.PACKET_HEARTBEAT;
            // timestamp
            pack_int32_to_buff(0, ref byte_data, 3);
            return net_proto_heartbeat_pack_len();
        }

        public static int net_proto_unpack_data(PacketType type, byte[] buf, int offset, int len, ref PacketUnpackData data)
        {
            int handled = 0;
            switch (type) {
                case PacketType.PACKET_USER_DATA:
                    {
                        int off = 0;
                        // msg id
                        data.param = unpack_int16_from_buff(buf, offset+off);
                        off += 2;
                        // msg info
                        data.msg_info.msg_id = data.param;
                        data.msg_info.buf = buf;
                        data.msg_info.offset = (ushort)(offset+off);
                        data.msg_info.len = (ushort)(len-data.msg_info.offset);
                    }
                    break;
                case PacketType.PACKET_HEARTBEAT:
                    {
                        if (len - offset < PACKET_LEN_TIMESTAMP)
                            return 0;
                        // time
                        data.param = unpack_int32_from_buff(buf, offset);
                        handled = PACKET_LEN_TIMESTAMP;
                    }
                    break;
                case PacketType.PACKET_DISCONNECT:
                case PacketType.PACKET_DISCONNECT_ACK:
                    {
                        handled = 0;
                    }
                    break;
                default:
                    Console.WriteLine("handle packet type:{0} failed", type);
                    return -1;
            }
            return handled;
        }
    }
}
