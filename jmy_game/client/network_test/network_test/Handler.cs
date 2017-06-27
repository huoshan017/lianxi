using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using Google.Protobuf;
using System.Net;

namespace network_test
{
    public class LoginHandler
    {
        public static int onConnect(string endpoint_str)
        {
            string evt_str = "connected login server: " + endpoint_str;
            Global.win.CallOutputInfo_dg(evt_str, main_window.OutputLevel.InfoLevel);
            Global.win.CallSetConnectButtonText_Disconnect_dg();
            return 0;
        }

        public static int onDisconnect(string endpoint_str)
        {
            string evt_str = "disconnected login server";
            Global.win.CallOutputInfo_dg(evt_str, main_window.OutputLevel.InfoLevel);
            Global.win.CallSetConnectButtonText_Connect_dg();
            Global.win.CallClearServerList_dg();
            return 0;
        }

        public static int onError(int err)
        {
            string evt_str = "";
            Global.win.CallOutputInfo_dg(evt_str, main_window.OutputLevel.ErrorLevel);
            return 0; 
        }

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
                string info = "message MsgS2C_LoginResponse merge from failed, err:" + e.Message;
                Console.WriteLine(info);
                Global.win.CallOutputInfo_dg(info, main_window.OutputLevel.WarnLevel);
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
            string info;
            try
            {
                response.MergeFrom(stream);
                Global.enter_session = System.Text.Encoding.Default.GetString(response.SessionCode.ToArray());
            }
            catch (Exception e)
            {
                info = "message MsgS2C_SelectServerResponse merge from failed, err: " + e.Message;
                Console.WriteLine(info);
                Global.win.CallOutputInfo_dg(info, main_window.OutputLevel.WarnLevel);
            }

            info = "get server_ip:" + System.Text.Encoding.Default.GetString(response.ServerIp.ToArray()) + ", port:" + response.Port + ", enter_session:" + System.Text.Encoding.Default.GetString(response.SessionCode.ToArray());
            Console.WriteLine(info);
            Global.win.CallOutputInfo_dg(info, main_window.OutputLevel.InfoLevel);

            AsynchronousClient game_client = Global.GetGameClient();
            game_client.Start(System.Text.Encoding.Default.GetString(response.ServerIp.ToArray()), (ushort)response.Port);
            Global.request.send_get_role_request(game_client, Global.account, System.Text.Encoding.Default.GetString(response.SessionCode.ToArray()));
            
            return 0;
        }
    }

    public class GameHandler
    {
        public static int onConnect(string endpoint_str)
        {
            string evt_str = "connected game server: " + endpoint_str;
            Global.win.CallOutputInfo_dg(evt_str, main_window.OutputLevel.InfoLevel);
            return 0;
        }

        public static int onDisconnect(string endpoint_str)
        {
            string evt_str = "disconnected game server";
            Global.win.CallOutputInfo_dg(evt_str, main_window.OutputLevel.InfoLevel);
            Global.win.CallClearServerList_dg();
            return 0;
        }

        public static int onError(int err)
        {
            string evt_str = "";
            Global.win.CallOutputInfo_dg(evt_str, main_window.OutputLevel.ErrorLevel);
            return 0;
        }

        public static int processError(net_proto.MsgInfo msg_info)
        {
            MsgError response = new MsgError();
            MemoryStream stream = new MemoryStream(msg_info.buf, msg_info.offset, msg_info.len);
            try
            {
                response.MergeFrom(stream);
            }
            catch (Exception e)
            {
                string info = "message MsgError merge from failed, err: " + e.Message;
                Console.WriteLine(info);
                Global.win.CallOutputInfo_dg(info, main_window.OutputLevel.WarnLevel);
            }

            if (response.ErrorCode == ProtoErrorType.ProtoErrorGetRoleNone)
            {
                Global.request.send_create_role_request(Global.GetGameClient(), "huoshan017", 0, 0);
            }
            else
            {
                string info = "not handled error code: " + response.ErrorCode;
                Console.WriteLine(info);
                Global.win.CallOutputInfo_dg(info, main_window.OutputLevel.WarnLevel);
            }
            return 0;
        }

        public static int processGetRole(net_proto.MsgInfo msg_info)
        {
            MsgS2C_GetRoleResponse response = new MsgS2C_GetRoleResponse();
            MemoryStream stream = new MemoryStream(msg_info.buf, msg_info.offset, msg_info.len);
            try
            {
                response.MergeFrom(stream);
            }
            catch (Exception e)
            {
                string info = "message MsgS2C_GetRoleResponse merge from failed, err: " + e.Message;
                Console.WriteLine(info);
                Global.win.CallOutputInfo_dg(info, main_window.OutputLevel.WarnLevel);
            }
            if (response.RoleData.RoleId == 0)
            {
                Global.request.send_create_role_request(Global.GetGameClient(), Global.account, 0, 0);
            }
            else
            {
                Global.request.send_enter_game_request(Global.GetGameClient(), Global.account, Global.enter_session);
            }
            return 0;
        }

        public static int processCreateRole(net_proto.MsgInfo msg_info)
        {
            MsgS2C_CreateRoleResponse response = new MsgS2C_CreateRoleResponse();
            MemoryStream stream = new MemoryStream(msg_info.buf, msg_info.offset, msg_info.len);
            string info;
            try
            {
                response.MergeFrom(stream);
            }
            catch (Exception e)
            {
                info = "message MsgS2C_CreateRoleResponse merge from failed, err: " + e.Message;
                Console.WriteLine(info);
                Global.win.CallOutputInfo_dg(info, main_window.OutputLevel.WarnLevel);
            }

            info = "create role success, id:" + response.RoleData.RoleId;
            Console.WriteLine(info);
            Global.win.CallOutputInfo_dg(info, main_window.OutputLevel.InfoLevel);

            Global.request.send_enter_game_request(Global.GetGameClient(), Global.account, Global.enter_session);
            return 0;
        }

        public static int processEnterGame(net_proto.MsgInfo msg_info)
        {
            MsgS2C_EnterGameResponse response = new MsgS2C_EnterGameResponse();
            MemoryStream stream = new MemoryStream(msg_info.buf, msg_info.offset, msg_info.len);
            string info;
            try
            {
                response.MergeFrom(stream);
            }
            catch (Exception e)
            {
                info = "message MsgS2C_EnterGameResponse merge from failed, err: " + e.Message;
                Console.WriteLine(info);
                Global.win.CallOutputInfo_dg(info, main_window.OutputLevel.WarnLevel);
            }
            info = "enter game";
            Console.WriteLine(info);
            Global.win.CallOutputInfo_dg(info, main_window.OutputLevel.InfoLevel);

            return 0;
        }

        public static int processEnterGameComplete(net_proto.MsgInfo msg_info)
        {
            MsgS2C_EnterGameCompleteNotify notify = new MsgS2C_EnterGameCompleteNotify();
            MemoryStream stream = new MemoryStream(msg_info.buf, msg_info.offset, msg_info.len);
            string info;
            try
            {
                notify.MergeFrom(stream);
            }
            catch (Exception e)
            {
                info = "message MsgS2C_EnterGameCompleteNotify merge from failed, err: " + e.Message;
                Console.WriteLine(info);
                Global.win.CallOutputInfo_dg(info, main_window.OutputLevel.WarnLevel);
            }
            info = "enter game complete";
            Console.WriteLine(info);
            Global.win.CallOutputInfo_dg(info, main_window.OutputLevel.InfoLevel);
            return 0;
        }

        public static int processChat(net_proto.MsgInfo msg_info)
        {
            return 0;
        }

        public static int processChatNotify(net_proto.MsgInfo msg_info)
        {
            return 0;
        }
    };
}
