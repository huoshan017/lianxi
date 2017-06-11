#pragma once

#include "../libjmy/jmy_const.h"
#include "../../proto/src/common.pb.h"

struct JmyMsgInfo;
struct JmyEventInfo;
class ClientHandler
{
public:
	static int onConnect(JmyEventInfo*);
	static int onDisconnect(JmyEventInfo*);
	static int onTick(JmyEventInfo*);

	static int processLogin(JmyMsgInfo*);
	static int processSelectServer(JmyMsgInfo*);
	static int processEcho(JmyMsgInfo*);

private:
	static int send_error(JmyMsgInfo* info, ProtoErrorType error);

private:
	static char tmp_[JMY_MAX_MSG_SIZE];
};
