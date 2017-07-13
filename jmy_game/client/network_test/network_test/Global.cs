using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Google.Protobuf;

namespace network_test
{
    class Global
    {
        public static mainwindow_mdi main_win;
        public static main_window win;
        private static AsynchronousClient login_client;
        private static AsynchronousClient game_client;
        public static Request request;
        public static string account;
        public static string enter_session;

        public static void Init()
        {
            main_win = new mainwindow_mdi();
            win = new main_window();
            request = new Request();
        }

        public static AsynchronousClient GetLoginClient()
        {
            if (login_client == null)
            {
                login_client = new AsynchronousClient();
                login_client.RegisterHandler((int)MsgIdType.MsgidS2CLoginResponse, LoginHandler.processLogin);
                login_client.RegisterHandler((int)MsgIdType.MsgidS2CSelectServerResponse, LoginHandler.processSelectServer);

                login_client.SetConnEventHandler(LoginHandler.onConnect);
                login_client.SetDisconnEventHandler(LoginHandler.onDisconnect);
                login_client.SetErrorEventHandler(LoginHandler.onError);
            }
            return login_client;
        }

        public static AsynchronousClient GetGameClient()
        {
            if (game_client == null)
            {
                game_client = new AsynchronousClient();
                game_client.RegisterHandler((int)MsgIdType.MsgidError, GameHandler.processError);
                game_client.RegisterHandler((int)MsgIdType.MsgidS2CGetRoleResponse, GameHandler.processGetRole);
                game_client.RegisterHandler((int)MsgIdType.MsgidS2CCreateRoleResponse, GameHandler.processCreateRole);
                game_client.RegisterHandler((int)MsgIdType.MsgidS2CEnterGameResponse, GameHandler.processEnterGame);
                game_client.RegisterHandler((int)MsgIdType.MsgidS2CEnterGameCompleteNotify, GameHandler.processEnterGameComplete);
                game_client.RegisterHandler((int)MsgIdType.MsgidS2CChatResponse, GameHandler.processChat);
                game_client.RegisterHandler((int)MsgIdType.MsgidS2CChatNotify, GameHandler.processChatNotify);

                game_client.SetConnEventHandler(GameHandler.onConnect);
                game_client.SetDisconnEventHandler(GameHandler.onDisconnect);
                game_client.SetErrorEventHandler(GameHandler.onError);
            }
            return game_client;
        }
    }
}
