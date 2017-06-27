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

    public enum ClientEvent { OutputEvent = 0, ConnectEvent = 1, DisconnectEvent = 2, ErrorEvent = 3 };

    public delegate int msg_handler(net_proto.MsgInfo msg_info);
    public delegate int conn_evt_handler(string endpoint_str);
    public delegate int disconn_evt_handler(string endpoint_str);
    public delegate int err_evt_handler(int err);

    public class AsynchronousClient
    {
        // socket
        private Socket client_;
        private LinkedList<byte[]> receive_list_;
        private LinkedList<byte[]> send_list_;
        private Dictionary<int, msg_handler> handlers_;
        private Thread worker_;

        public enum NetState { NotConnected = 0, Connecting = 1, Connected = 2 };
        private NetState state_ = NetState.NotConnected;
        private bool is_receiving_ = false;
        private bool is_sending_ = false;
        private net_proto.PacketUnpackData d;
        private conn_evt_handler connect_event_handler_ = null;
        private disconn_evt_handler disconnect_event_handler_ = null;
        private err_evt_handler error_event_handler_ = null;
        private int to_close_ = 0;

        public AsynchronousClient()
        {
            handlers_ = new Dictionary<int, msg_handler>();
            receive_list_ = new LinkedList<byte[]>();
            send_list_ = new LinkedList<byte[]>();
        }

        public NetState getState()
        {
            return state_;
        }

        public bool RegisterHandler(int msg_id, msg_handler handler) {
            handlers_.Add(msg_id, handler);
            return true;
        }

        public void SetConnEventHandler(conn_evt_handler handler)
        {
            connect_event_handler_ = handler;
        }

        public void SetDisconnEventHandler(disconn_evt_handler handler)
        {
            disconnect_event_handler_ = handler;
        }

        public void SetErrorEventHandler(err_evt_handler handler)
        {
            error_event_handler_ = handler;
        }

        public void Start(string ip, ushort port)
        {
            // Connect to a remote device.     
            try
            {
                // Establish the remote endpoint for the socket.     
                // The name of the
                IPAddress ipAddress = IPAddress.Parse(ip);
                IPEndPoint remoteEP = new IPEndPoint(ipAddress, port);
                // Create a TCP/IP socket.     
                client_ = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
                // Connect to the remote endpoint.     
                client_.BeginConnect(remoteEP, new AsyncCallback(ConnectCallback), client_);
                state_ = NetState.Connecting;

                worker_ = new Thread(ThreadFunc);
                worker_.Start();
            }
            catch (Exception e)
            {
                Console.WriteLine(e.ToString());
            }
        }

        public void Close()
        {
            Interlocked.CompareExchange(ref to_close_, 1, 0);
        }

        private void RealClose()
        {
            if (state_ != NetState.Connected)
                return;
            // Release the socket.     
            client_.Shutdown(SocketShutdown.Both);
            client_.Close();
            if (receive_list_ != null)
                receive_list_.Clear();
            if (send_list_ != null)
                send_list_.Clear();
            is_receiving_ = false;
            is_sending_ = false;
            state_ = NetState.NotConnected;
            if (disconnect_event_handler_ != null)
            {
                disconnect_event_handler_("");
            }
            Interlocked.CompareExchange(ref to_close_, 0, 1);
            worker_.Join();
        }

        public void ThreadFunc() {
            while (true)
            {
                if (Run() == 0)
                    break;

                Thread.Sleep(100);
            }
            RealClose();
            Console.WriteLine("线程退出");
        }

        public int Run()
        {
            if (state_ == NetState.Connected)
            {
                if (to_close_ != 0)
                {
                    return 0;
                }
                
                // Receive the response from the remote device.
                if (!is_receiving_)
                {
                    StartReceive();
                    is_receiving_ = true;
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
            else if (state_ == NetState.NotConnected)
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

            lock ( send_list_ )
            {
                send_list_.AddLast(msg);
            }
            return 1;
        }

        private int HandleOne(byte[] data)
        {
            if (net_proto.net_proto_unpack_data(net_proto.PacketType.PACKET_USER_DATA, data, 0, data.Length, ref d) < 0)
            {
                return -1;
            }

            if (!handlers_.ContainsKey(d.msg_info.msg_id))
            {
                Console.WriteLine("找不到消息{0}的处理函数", d.msg_info.msg_id);
                return 0;
            }

            msg_handler handle = handlers_[d.msg_info.msg_id];
            if (handle != null) {
                if (handle(d.msg_info) < 0)
                {
                    Console.WriteLine("处理消息{0}失败", d.msg_info.msg_id);
                    return -1;
                }
            }
            return 1;
        }

        private int ProcessData()
        {
            if (receive_list_.Count() <= 0)
                return 0;

            int res = 0;
            lock (receive_list_)
            {
                foreach (byte[] n in receive_list_)
                {
                    if (HandleOne(n) < 0)
                    {
                        res = -1;
                        break;
                    }
                }
                receive_list_.Clear();
            }
            return res;
        }

        private int HandleSend()
        {
            if (is_sending_)
                return 0;

            if (send_list_.Count() <= 0)
                return 0;

            byte[] n = null;
            lock (send_list_)
            {
                if (send_list_.Count() <= 0)
                    return 0;

                n = send_list_.First();
                if (n == null)
                    return 0;
            }

            Send(n);

            is_sending_ = true;
            return 0;
        }

        private void ConnectCallback(IAsyncResult ar)
        {
            try
            {
                // Retrieve the socket from the state object.     
                Socket client = (Socket)ar.AsyncState;
                // Complete the connection.     
                client.EndConnect(ar);
                
                state_ = NetState.Connected;
                send_list_ = new LinkedList<byte[]>();
                receive_list_ = new LinkedList<byte[]>();

                connect_event_handler_(client.RemoteEndPoint.ToString());
            }
            catch (Exception e)
            {
                Console.WriteLine(e.ToString());
            }
        }
        private void StartReceive()
        {
            if (to_close_ != 0)
                return;

            try
            {
                // Create the state object.     
                StateObject state = new StateObject();
                state.workSocket = client_;
                ushort buf_len = net_proto.PACKET_LEN_HEAD + net_proto.PACKET_LEN_TYPE;
                state.buffer = new byte[buf_len];
                state.buffer_size = buf_len;
                // Begin receiving the data from the remote device.     
                client_.BeginReceive(state.buffer, 0, buf_len, 0, new AsyncCallback(ReceiveCallback), state);
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
            if (to_close_ != 0)
                return;

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

                    // Get the rest of the data.     
                    RecvPacket(type, (ushort)len);
                }
                else
                {

                }
            }
            catch (Exception e)
            {
                Console.WriteLine(e.ToString());
            }
        }

        private void RecvPacket(net_proto.PacketType type, ushort byte_size)
        {
            if (to_close_ != 0)
                return;

            try
            {
                StateObject state = new StateObject();
                state.workSocket = client_;
                state.buffer = new byte[byte_size];
                state.buffer_size = byte_size;
                // Begin receiving the data from the remote device.     
                client_.BeginReceive(state.buffer, 0, byte_size, 0, new AsyncCallback(ReceivePacketCallback), state);
            }
            catch (Exception e)
            {
                Console.WriteLine(e.ToString());
            }
        }

        private void ReceivePacketCallback(IAsyncResult ar)
        {
            if (to_close_ != 0)
                return;

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
                    lock (receive_list_)
                    {
                        receive_list_.AddLast(state.buffer);
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
            if (to_close_ != 0)
                return;

            try
            {
                // Begin sending the data to the remote device.     
                client_.BeginSend(byteData, 0, byteData.Length, 0, new AsyncCallback(SendCallback), client_);
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
            if (to_close_ != 0)
                return;

            try
            {
                // Retrieve the socket from the state object.     
                Socket client = (Socket)ar.AsyncState;
                // Complete sending the data to the remote device.     
                int bytesSent = client.EndSend(ar);

                byte[] n = null;
                lock (send_list_)
                {
                    if (send_list_.First() == null)
                    {
                        Console.WriteLine("not found send list first node");
                        return;
                    }

                    send_list_.RemoveFirst();
                    if (send_list_.Count() <= 0)
                    {
                        is_sending_ = false;
                        return;
                    }

                    n = send_list_.First();
                    if (n == null)
                        is_sending_ = false;
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