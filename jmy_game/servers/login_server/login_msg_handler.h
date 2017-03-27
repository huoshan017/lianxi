#pragma once

struct JmyMsgInfo;
class LoginMsgHandler
{
public:
	static int processLogin(JmyMsgInfo*);
	static int processSelectGameServer(JmyMsgInfo*);
	static int processEnterGameServer(JmyMsgInfo*);
};
