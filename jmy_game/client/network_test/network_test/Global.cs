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
        public static AsynchronousClient client;
        public static Request request;

        public static void Init()
        {
            win = new main_window();
            client = new AsynchronousClient();
            request = new Request();
            client.RegisterHandler((int)MsgIdType.MsgidS2CLoginResponse, Handler.processLogin);
            client.RegisterHandler((int)MsgIdType.MsgidS2CSelectServerResponse, Handler.processSelectServer);
        }
    }
}
