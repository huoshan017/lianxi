using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using Google.Protobuf;

namespace network_test
{
    public class Request
    {
        public Request()
        {
            
        }

        public int send_login_request(AsynchronousClient client, string account, string password)
        {
            MsgC2S_LoginRequest req = new MsgC2S_LoginRequest();
            req.Account = ByteString.CopyFrom(System.Text.Encoding.Default.GetBytes(account));
            req.Password = ByteString.CopyFrom(System.Text.Encoding.Default.GetBytes(password));
            MemoryStream stream = new MemoryStream();
            req.WriteTo(stream);
            byte[] out_bytes = stream.ToArray();
            client.SendMsg((int)MsgIdType.MsgidC2SLoginRequest, out_bytes, 0, (ushort)out_bytes.Length);
            return 0;
        }

        public int send_select_server(AsynchronousClient client, int game_server_id)
        {
            MsgC2S_SelectServerRequest req = new MsgC2S_SelectServerRequest();
            req.SelId = game_server_id;
            MemoryStream stream = new MemoryStream();
            req.WriteTo(stream);
            byte[] out_bytes = stream.ToArray();
            client.SendMsg((int)MsgIdType.MsgidC2SSelectServerRequest, out_bytes, 0, (ushort)out_bytes.Length);
            return 0;
        }
    }
}
