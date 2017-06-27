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

        public int send_select_server_request(AsynchronousClient client, int game_server_id)
        {
            MsgC2S_SelectServerRequest req = new MsgC2S_SelectServerRequest();
            req.SelId = game_server_id;
            MemoryStream stream = new MemoryStream();
            req.WriteTo(stream);
            byte[] out_bytes = stream.ToArray();
            client.SendMsg((int)MsgIdType.MsgidC2SSelectServerRequest, out_bytes, 0, (ushort)out_bytes.Length);
            return 0;
        }

        public int send_get_role_request(AsynchronousClient client, string account, string session)
        {
            MsgC2S_GetRoleRequest req = new MsgC2S_GetRoleRequest();
            req.Account = ByteString.CopyFrom(System.Text.Encoding.Default.GetBytes(account));
            req.EnterSession = ByteString.CopyFrom(System.Text.Encoding.Default.GetBytes(session));
            MemoryStream stream = new MemoryStream();
            req.WriteTo(stream);
            byte[] out_bytes = stream.ToArray();
            client.SendMsg((int)MsgIdType.MsgidC2SGetRoleRequest, out_bytes, 0, (ushort)out_bytes.Length);
            return 0;
        }

        public int send_create_role_request(AsynchronousClient client, string nick_name, int sex, int race)
        {
            MsgC2S_CreateRoleRequest req = new MsgC2S_CreateRoleRequest();
            req.NickName = ByteString.CopyFrom(System.Text.Encoding.Default.GetBytes(nick_name));
            req.Sex = sex;
            req.Race = race;
            MemoryStream stream = new MemoryStream();
            req.WriteTo(stream);
            byte[] out_bytes = stream.ToArray();
            client.SendMsg((int)MsgIdType.MsgidC2SCreateRoleRequest, out_bytes, 0, (ushort)out_bytes.Length);
            return 0;
        }

        public int send_enter_game_request(AsynchronousClient client, string account, string enter_session)
        {
            MsgC2S_EnterGameRequest req = new MsgC2S_EnterGameRequest();
            MemoryStream stream = new MemoryStream();
            req.WriteTo(stream);
            byte[] out_bytes = stream.ToArray();
            client.SendMsg((int)MsgIdType.MsgidC2SEnterGameRequest, out_bytes, 0, (ushort)out_bytes.Length);
            return 0;
        }

        public int send_gm_cmd_request(AsynchronousClient client, string cmd)
        {
            MsgC2S_ChatRequest req = new MsgC2S_ChatRequest();
            req.Content = ByteString.CopyFrom(System.Text.Encoding.Default.GetBytes(cmd));
            MemoryStream stream = new MemoryStream();
            req.WriteTo(stream);
            byte[] out_bytes = stream.ToArray();
            client.SendMsg((int)MsgIdType.MsgidC2SChatRequest, out_bytes, 0, (ushort)out_bytes.Length);
            return 0;
        }
    }
}
