using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net;
using System.Net.Sockets;

namespace network_test
{
    class Program
    {
        static void Main(string[] args)
        {
            IPEndPoint ep = new IPEndPoint(IPAddress.Parse("127.0.0.1"), 10000);
            TcpClient client = new TcpClient();
            client.Connect(ep);
        }
    }
}
