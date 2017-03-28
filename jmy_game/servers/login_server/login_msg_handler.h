#pragma once

#include "../libjmy/jmy_const.h"
#include "../../proto/common.pb.h"

struct JmyMsgInfo;
class JmyTcpConnection;
class LoginMsgHandler
{
public:
	static int processLogin(JmyMsgInfo*);
	static int processSelectServer(JmyMsgInfo*);
	static int processEnterGame(JmyMsgInfo*);

private:
	static void send_login_error(JmyTcpConnection* conn, MsgL2CLoginResponse& response, int error);
private:
	static char tmp_[MAX_SEND_BUFFER_SIZE];
};
