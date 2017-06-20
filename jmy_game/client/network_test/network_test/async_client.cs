using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net;
using System.Net.Sockets;
using System.Threading;

namespace network_test
{
    // State object for receiving data from remote device.     
    public class StateObject
    {
        // Client socket.     
        public Socket workSocket = null;
        // Size of receive buffer.     
        public ushort buffer_size = 0;
        // Receive buffer.     
        public byte[] buffer = null;
        // Received data string.     
        public StringBuilder sb = new StringBuilder();
    }

    public delegate int msg_handler(net_proto.MsgInfo msg_info);

    public class AsynchronousClient
    {
        // socket
        private Socket client;
        private LinkedList<byte[]> receive_list;
        private LinkedList<byte[]> send_list;
        private Dictionary<int, msg_handler> handlers;
        private Thread worker;

        // ManualResetEvent instances signal completion.
        //private static ManualResetEvent connectDone = new ManualResetEvent(false);
        //private static ManualResetEvent sendDone = new ManualResetEvent(false);
        //private static ManualResetEvent receiveDone = new ManualResetEvent(false);
        // The response from the remote device.

        private static String response = String.Empty;
        enum NetState { NotConnected = 0, Connecting = 1, Connected = 2 };
        private NetState state = NetState.NotConnected;
        private bool is_receiving = false;
        private bool is_sending = false;
        net_proto.PacketUnpackData d;

        public bool RegisterHandler(int msg_id, msg_handler handler) {
            if (handlers == null) {
                handlers = new Dictionary<int, msg_handler>();
            }
            if (handlers[msg_id] != null) {
                return false;
            }
            handlers[msg_id] = handler;
            return true;
        }

        public void StartClient(string ip, ushort port)
        {
            // Connect to a remote device.     
            try
            {
                // Establish the remote endpoint for the socket.     
                // The name of the
                IPAddress ipAddress = IPAddress.Parse(ip);
                IPEndPoint remoteEP = new IPEndPoint(ipAddress, port);
                // Create a TCP/IP socket.     
                client = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
                // Connect to the remote endpoint.     
                client.BeginConnect(remoteEP, new AsyncCallback(ConnectCallback), client);
                state = NetState.Connecting;

                worker = new Thread(ThreadFunc);
                worker.Start();
            }
            catch (Exception e)
            {
                Console.WriteLine(e.ToString());
            }
        }

        public void ThreadFunc() {
            while (true)
            {
                if (Run() == 0)
                    break;

                Thread.Sleep(100);
            }
            client.Close();
            Console.WriteLine("线程退出");
        }

        public int Run()
        {
            if (state == NetState.Connected)
            {
                // Receive the response from the remote device.
                if (!is_receiving)
                {
                    StartReceive();
                    is_receiving = true;
                }

                if (HandleSend() < 0)
                {
                    return -1;
                }

                if (ProcessData() < 0)
                {
                    return -1;
                }
            }
            else if (state == NetState.NotConnected)
                return 0;

            return 1;
        }

        public int SendMsg(int msg_id, byte[] byteData, int offset, ushort data_len)
        {
            byte[] msg = new byte[net_proto.net_proto_user_data_pack_len(data_len)];
            int head_len = net_proto.net_proto_pack_user_data_head(ref msg, msg_id, data_len);
            if (head_len < 0)
            {
                Console.WriteLine("pack user data head for msg {0} failed", msg_id);
                return -1;
            }
            byteData.CopyTo(msg, head_len);

            lock ( send_list )
            {
                send_list.AddLast(msg);
            }
            return 1;
        }

        private int HandleOne(byte[] data)
        {
            if (net_proto.net_proto_unpack_data(net_proto.PacketType.PACKET_USER_DATA, data, 0, data.Length, ref d) < 0)
            {
                return -1;
            }

            msg_handler handle = handlers[d.msg_info.msg_id];
            if (handle != null) {
                if (handle(d.msg_info) < 0)
                    return -1;
            }
            return 0;
        }

        private int ProcessData()
        {
            lock (receive_list)
            {
                foreach (byte[] n in receive_list)
                {
                    if (HandleOne(n) < 0)
                    {
                        return -1;
                    }
                }
            }
            return 0;
        }

        private int HandleSend()
        {
            if (is_sending)
                return 0;

            byte[] n = null;
            lock (send_list)
            {
                if (send_list.Count() <= 0)
                    return 0;

                n = send_list.First();
                if (n == null)
                    return 0;
            }

            Send(n);

            is_sending = true;
            return 0;
        }

        public void Close()
        {
            // Release the socket.     
            client.Shutdown(SocketShutdown.Both);
            client.Close();
            is_receiving = false;
            state = NetState.NotConnected;
            worker.Join();
        }

        private void ConnectCallback(IAsyncResult ar)
        {
            try
            {
                // Retrieve the socket from the state object.     
                Socket client = (Socket)ar.AsyncState;
                // Complete the connection.     
                client.EndConnect(ar);
                Console.WriteLine("Socket connected to {0}", client.RemoteEndPoint.ToString());
                state = NetState.Connected;
                send_list = new LinkedList<byte[]>();
                receive_list = new LinkedList<byte[]>();
            }
            catch (Exception e)
            {
                Console.WriteLine(e.ToString());
            }
        }
        private void StartReceive()
        {
            try
            {
                // Create the state object.     
                StateObject state = new StateObject();
                state.workSocket = client;
                ushort buf_len = net_proto.PACKET_LEN_HEAD + net_proto.PACKET_LEN_TYPE;
                state.buffer = new byte[buf_len];
                state.buffer_size = buf_len;
                // Begin receiving the data from the remote device.     
                client.BeginReceive(state.buffer, 0, buf_len, 0, new AsyncCallback(ReceiveCallback), state);
            }
            catch (Exception e)
            {
                Console.WriteLine(e.ToString());
            }
        }
        private int HandlePacketType(net_proto.PacketType type)
        {
            if (type == net_proto.PacketType.PACKET_DISCONNECT)
            {
            }
            else if (type == net_proto.PacketType.PACKET_DISCONNECT_ACK)
            {
            }
            else if (type == net_proto.PacketType.PACKET_HEARTBEAT)
            {
            }
            else if (type == net_proto.PacketType.PACKET_USER_DATA)
            {
                return 1;
            }
            return 0;
        }
        private void ReceiveCallback(IAsyncResult ar)
        {
            try
            {
                // Retrieve the state object and the client socket     
                // from the asynchronous state object.     
                StateObject state = (StateObject)ar.AsyncState;
                Socket client = state.workSocket;
                // Read data from the remote device.     
                int bytesRead = client.EndReceive(ar);
                if (bytesRead > 0)
                {
                    // There might be more data, so store the data received so far.
                    net_proto.PacketType type = net_proto.PacketType.PACKET_NONE;
                    int len = 0;
                    if (!net_proto.net_proto_get_packet_len_type(state.buffer, ref type, ref len)) {
                        client.Close();
                        Console.WriteLine("get packet type and len failed");
                        return;
                    }
                    if (HandlePacketType(type) == 0) {
                        StartReceive();
                        return;
                    }

                    byte[] b = new byte[len];
                    state.buffer = b;
                    state.buffer_size = (ushort)len;
                    // Get the rest of the data.     
                    client.BeginReceive(state.buffer, 0, state.buffer_size, 0, new AsyncCallback(ReceiveCallback), state);
                }
                else
                {
                    // All the data has arrived; put it in response.     
                    if (state.sb.Length > 1)
                    {
                        response = state.sb.ToString();
                        // Write the response to the console.     
                        Console.WriteLine("Response received : {0}", response);
                    }
                }
            }
            catch (Exception e)
            {
                Console.WriteLine(e.ToString());
            }
        }

        private void RecvPacket(net_proto.PacketType type, ushort byte_size)
        {
            try
            {
                StateObject state = new StateObject();
                state.workSocket = client;
                state.buffer = new byte[byte_size];
                state.buffer_size = byte_size;
                // Begin receiving the data from the remote device.     
                client.BeginReceive(state.buffer, 0, byte_size, 0, new AsyncCallback(ReceivePacketCallback), state);
            }
            catch (Exception e)
            {
                Console.WriteLine(e.ToString());
            }
        }

        private void ReceivePacketCallback(IAsyncResult ar)
        {
            try
            {
                // Retrieve the state object and the client socket     
                // from the asynchronous state object.     
                StateObject state = (StateObject)ar.AsyncState;
                Socket client = state.workSocket;
                // Read data from the remote device.     
                int bytesRead = client.EndReceive(ar);
                if (bytesRead > 0)
                {
                    lock (receive_list)
                    {
                        receive_list.AddLast(state.buffer);
                    }
                    StartReceive();
                }
                else
                {
                    Console.WriteLine("receive packet no read bytes");
                }
            }
            catch (Exception e)
            {
                Console.WriteLine(e.ToString());
            }
        }

        private void Send(byte[] byteData)
        {
            try
            {
                // Begin sending the data to the remote device.     
                client.BeginSend(byteData, 0, byteData.Length, 0, new AsyncCallback(SendCallback), client);
            }
            catch (Exception e)
            {
                Console.WriteLine(e.ToString());
            }
        }

        private void Send(String data)
        {
            // Convert the string data to byte data using ASCII encoding.     
            byte[] byteData = Encoding.ASCII.GetBytes(data);
            Send(byteData);
        }

        private void SendCallback(IAsyncResult ar)
        {
            try
            {
                // Retrieve the socket from the state object.     
                Socket client = (Socket)ar.AsyncState;
                // Complete sending the data to the remote device.     
                int bytesSent = client.EndSend(ar);

                byte[] n = null;
                lock (send_list)
                {
                    if (send_list.First() == null)
                    {
                        Console.WriteLine("not found send list first node");
                        return;
                    }

                    send_list.RemoveFirst();
                    n = send_list.First();
                    if (n == null)
                        is_sending = false;
                }

                Send(n);
                Console.WriteLine("Sent {0} bytes to server.", bytesSent);;
            }
            catch (Exception e)
            {
                Console.WriteLine(e.ToString());
            }
        }
    }
}