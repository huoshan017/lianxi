using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using Google.Protobuf;

namespace network_test
{
    public class Handler
    {
        public static int processLogin(net_proto.MsgInfo msg_info)
        {
            MsgS2C_LoginResponse response = new MsgS2C_LoginResponse();
            MemoryStream stream = new MemoryStream();
            stream.Read(msg_info.buf, msg_info.offset, msg_info.len);
            response.MergeFrom(stream);
            foreach (MsgServerInfo s in response.Servers) {
                Global.win.InsertServerInfo(s.Id, s.Name.ToString());
            }
            return 0;
        }

        public static int processSelectServer(net_proto.MsgInfo msg_info)
        {
            return 0;
        }
    }
}
