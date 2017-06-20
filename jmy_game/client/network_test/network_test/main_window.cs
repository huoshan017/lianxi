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
        public AsynchronousClient client;
        private Button button;
        private TextBox textbox;

        /// <summary>
        /// 覆盖OnLoad方法, 处理窗体第一次被显示时的消息
        /// </summary>
        protected override void OnLoad(EventArgs e)
        {
            base.OnLoad(e);
            // 设置按钮的位置
            this.button.Top = (this.Height - this.button.Height) / 2;
            this.button.Left = (this.Width - this.button.Width) / 2;
         }

        /// <summary>
        /// 委托方法, 处理按钮被点击事件
        /// </summary>
        private void ButtonClick(object sender, EventArgs e)
        {
            // 显示一个消息对话框
            MessageBox.Show("Hello");
            // 将文本框控件的内容该为Hello
            this.textbox.Text = "Hello";
        }

        public main_window()
        {
            InitializeComponent();
            this.button = new Button();
            this.button.Text = "Hello";
            this.button.Width = 120;
            this.button.Height = 90;
            // 为按钮对象的Click事件增加委托方法
            this.button.Click += new EventHandler(ButtonClick);

            // 初始化文本框对象
            this.textbox = new TextBox();
            this.textbox.Left = 10;
            this.textbox.Top = 10;

            // 将文本框加入到窗体上
            this.Controls.Add(this.textbox);
            // 实例化一个MyButton按钮对象并加入到窗体上
            this.Controls.Add(this.button);
        }

        private void main_window_Load(object sender, EventArgs e)
        {
            client = new AsynchronousClient();
            client.StartClient("192.168.3.250", 10000);
        }
    }
}
