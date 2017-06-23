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
        public static main_window win;
        public static AsynchronousClient login_client;
        public static AsynchronousClient game_client;
        public static Request request;
        public static string account;

        public static void Init()
        {
            win = new main_window();
            login_client = new AsynchronousClient();
            game_client = new AsynchronousClient();
            request = new Request();
            login_client.RegisterHandler((int)MsgIdType.MsgidS2CLoginResponse, Handler.processLogin);
            login_client.RegisterHandler((int)MsgIdType.MsgidS2CSelectServerResponse, Handler.processSelectServer);
        }

        public static AsynchronousClient GetLoginClient()
        {
            if (login_client == null)
            {
                login_client = new AsynchronousClient();
            }
            return login_client;
        }

        public static AsynchronousClient GetGameClient()
        {
            if (game_client == null)
            {
                game_client = new AsynchronousClient();
            }
            return game_client;
        }
    }
}
