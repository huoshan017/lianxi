using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Collections;

namespace network_test
{
    public partial class main_window : Form
    {
        private ArrayList game_id_array = null;
        /// <summary>
        /// 覆盖OnLoad方法, 处理窗体第一次被显示时的消息
        /// </summary>
        protected override void OnLoad(EventArgs e)
        {
            game_id_array = new ArrayList();
            base.OnLoad(e);
         }

        public main_window()
        {
            InitializeComponent();
        }

        private void main_window_Load(object sender, EventArgs e)
        {
        }

        private void connect_Click(object sender, EventArgs e)
        {
            string ip_str = ((TextBox)(this.Controls.Find("server_ip", false)[0])).Text;
            string port = ((TextBox)(this.Controls.Find("port", false)[0])).Text; ;
            Global.login_client.StartClient(ip_str, Convert.ToUInt16(port));
        }

        private void login_Click(object sender, EventArgs e)
        {
            string account = ((TextBox)(this.Controls.Find("account", false)[0])).Text;
            string password = ((TextBox)(this.Controls.Find("password", false)[0])).Text;
            Global.request.send_login_request(Global.login_client, account, password);
            Global.account = account;
        }

        public void InsertServerInfo(int game_server_id, string game_server_name)
        {
            ListBox list_box = ((ListBox)(this.Controls.Find("server_list", false)[0]));
            string item_text = game_server_name + ": " + game_server_id;
            game_id_array.Add(game_server_id);
            list_box.Items.Add(item_text);
        }

        private delegate void insert_server_info_d(int game_server_id, string game_server_name);

        public void CallInsertServerInfo_dg(int game_server_id, string game_server_name)
        {
            this.Invoke(new insert_server_info_d(InsertServerInfo), new object[]{ game_server_id, game_server_name });
        }

        private void send_chat_Click(object sender, EventArgs e)
        {
            TextBox chat_textbox = ((TextBox)(this.Controls.Find("chat_text", false)[0]));
            Global.request.send_gm_cmd_request(Global.login_client, chat_textbox.Text);
        }

        private void server_list_MouseClick(object sender, MouseEventArgs e)
        {
            ListBox server_list_box = ((ListBox)(this.Controls.Find("server_list", false)[0]));
            int index = server_list_box.IndexFromPoint(e.X, e.Y);
            int game_id = (int)game_id_array[index];
            Global.request.send_select_server_request(Global.login_client, game_id);
        }
    }
}
