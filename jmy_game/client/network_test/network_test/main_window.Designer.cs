namespace network_test
{
    partial class main_window
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.server_ip = new System.Windows.Forms.TextBox();
            this.connect = new System.Windows.Forms.Button();
            this.port = new System.Windows.Forms.TextBox();
            this.server_list = new System.Windows.Forms.ListView();
            this.account = new System.Windows.Forms.TextBox();
            this.password = new System.Windows.Forms.TextBox();
            this.login = new System.Windows.Forms.Button();
            this.account_label = new System.Windows.Forms.Label();
            this.password_label = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // server_ip
            // 
            this.server_ip.Location = new System.Drawing.Point(12, 12);
            this.server_ip.Name = "server_ip";
            this.server_ip.Size = new System.Drawing.Size(111, 21);
            this.server_ip.TabIndex = 0;
            this.server_ip.Text = "192.168.3.250";
            // 
            // connect
            // 
            this.connect.Location = new System.Drawing.Point(183, 10);
            this.connect.Name = "connect";
            this.connect.Size = new System.Drawing.Size(75, 23);
            this.connect.TabIndex = 1;
            this.connect.Text = "连接";
            this.connect.UseVisualStyleBackColor = true;
            this.connect.Click += new System.EventHandler(this.connect_Click);
            // 
            // port
            // 
            this.port.Location = new System.Drawing.Point(129, 12);
            this.port.Name = "port";
            this.port.Size = new System.Drawing.Size(48, 21);
            this.port.TabIndex = 2;
            this.port.Text = "10000";
            // 
            // server_list
            // 
            this.server_list.Location = new System.Drawing.Point(12, 49);
            this.server_list.Name = "server_list";
            this.server_list.Size = new System.Drawing.Size(165, 326);
            this.server_list.TabIndex = 3;
            this.server_list.UseCompatibleStateImageBehavior = false;
            // 
            // account
            // 
            this.account.Location = new System.Drawing.Point(338, 12);
            this.account.Name = "account";
            this.account.Size = new System.Drawing.Size(100, 21);
            this.account.TabIndex = 4;
            this.account.Text = "test1";
            // 
            // password
            // 
            this.password.Location = new System.Drawing.Point(491, 12);
            this.password.Name = "password";
            this.password.Size = new System.Drawing.Size(100, 21);
            this.password.TabIndex = 5;
            // 
            // login
            // 
            this.login.Location = new System.Drawing.Point(607, 12);
            this.login.Name = "login";
            this.login.Size = new System.Drawing.Size(75, 23);
            this.login.TabIndex = 6;
            this.login.Text = "登录";
            this.login.UseVisualStyleBackColor = true;
            this.login.Click += new System.EventHandler(this.login_Click);
            // 
            // account_label
            // 
            this.account_label.AutoSize = true;
            this.account_label.Location = new System.Drawing.Point(304, 17);
            this.account_label.Name = "account_label";
            this.account_label.Size = new System.Drawing.Size(29, 12);
            this.account_label.TabIndex = 7;
            this.account_label.Text = "账号";
            // 
            // password_label
            // 
            this.password_label.AutoSize = true;
            this.password_label.Location = new System.Drawing.Point(455, 17);
            this.password_label.Name = "password_label";
            this.password_label.Size = new System.Drawing.Size(29, 12);
            this.password_label.TabIndex = 8;
            this.password_label.Text = "密码";
            // 
            // main_window
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(715, 387);
            this.Controls.Add(this.password_label);
            this.Controls.Add(this.account_label);
            this.Controls.Add(this.login);
            this.Controls.Add(this.password);
            this.Controls.Add(this.account);
            this.Controls.Add(this.server_list);
            this.Controls.Add(this.connect);
            this.Controls.Add(this.server_ip);
            this.Controls.Add(this.port);
            this.Name = "main_window";
            this.Text = "main_window";
            this.Load += new System.EventHandler(this.main_window_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox server_ip;
        private System.Windows.Forms.Button connect;
        private System.Windows.Forms.TextBox port;
        private System.Windows.Forms.ListView server_list;
        private System.Windows.Forms.TextBox account;
        private System.Windows.Forms.TextBox password;
        private System.Windows.Forms.Button login;
        private System.Windows.Forms.Label account_label;
        private System.Windows.Forms.Label password_label;
    }
}