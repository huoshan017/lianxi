﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net;
using System.Net.Sockets;
using System.Threading;

namespace network_test
{
    class Program
    {
        static void Main(string[] args)
        {
            AsynchronousClient.StartClient();
        }
    }
}