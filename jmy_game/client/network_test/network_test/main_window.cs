using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace network_test
{
    public partial class main_window : Form
    {
        /// <summary>
        /// 覆盖OnLoad方法, 处理窗体第一次被显示时的消息
        /// </summary>
        protected override void OnLoad(EventArgs e)
        {
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
            Global.client = new AsynchronousClient();
            Global.client.StartClient(ip_str, Convert.ToUInt16(port));
        }

        private void login_Click(object sender, EventArgs e)
        {
            string account = ((TextBox)(this.Controls.Find("account", false)[0])).Text;
            string password = ((TextBox)(this.Controls.Find("password", false)[0])).Text;
            Global.request.send_login_request(Global.client, account, password);
        }

        public void InsertServerInfo(int game_server_id, string game_server_name)
        {
            ListView view = ((ListView)(this.Controls.Find("server_list", false)[0]));
            ListViewItem item = new ListViewItem();
            item.Text = game_server_name;
            item.Tag = game_server_id;
            view.Items.Add(item);
        }
    }
}
