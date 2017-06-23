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
            MemoryStream stream = new MemoryStream(msg_info.buf, msg_info.offset, msg_info.len);
            //stream.Read(msg_info.buf, msg_info.offset, msg_info.len);
            try
            {
                response.MergeFrom(stream);
            }
            catch (Exception e) {
                Console.WriteLine("message MsgS2C_LoginResponse merge from failed, err:{0}", e.Message);
            }

            foreach (MsgServerInfo s in response.Servers) {
                Global.win.CallInsertServerInfo_dg(s.Id, System.Text.Encoding.Default.GetString(s.Name.ToArray()));
            }
            return 0;
        }

        public static int processSelectServer(net_proto.MsgInfo msg_info)
        {
            MsgS2C_SelectServerResponse response = new MsgS2C_SelectServerResponse();
            MemoryStream stream = new MemoryStream(msg_info.buf, msg_info.offset, msg_info.len);
            try
            {
                response.MergeFrom(stream);
            }
            catch (Exception e)
            {
                Console.WriteLine("message MsgS2C_SelectServerResponse merge from failed, err: {0}", e.Message);
            }

            Console.WriteLine("get server_ip:{0}, port:{1}, enter_session:{2}", System.Text.Encoding.Default.GetString(response.ServerIp.ToArray()), response.Port, System.Text.Encoding.Default.GetString(response.SessionCode.ToArray()));

            AsynchronousClient game_client = Global.GetGameClient();
            game_client.StartClient(System.Text.Encoding.Default.GetString(response.ServerIp.ToArray()), (ushort)response.Port);
            Global.request.send_get_role_request(game_client, Global.account, System.Text.Encoding.Default.GetString(response.SessionCode.ToArray()));
            return 0;
        }
    }
}
