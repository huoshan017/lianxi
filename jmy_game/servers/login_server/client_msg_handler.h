#pragma once

#include "../libjmy/jmy_const.h"
#include "../../proto/src/common.pb.h"
#include "../../proto/src/error.pb.h"

struct JmyMsgInfo;
class JmyTcpConnection;
class ClientMsgHandler
{
public:
	static int processLogin(JmyMsgInfo*);
	static int processSelectServer(JmyMsgInfo*);

private:
	static void send_error(JmyMsgInfo* info, ProtoErrorType error);
private:
	static char tmp_[MAX_SEND_BUFFER_SIZE];
};
