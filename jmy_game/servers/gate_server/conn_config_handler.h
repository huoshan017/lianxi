#pragma once

#include "../libjmy/jmy_const.h"

struct JmyMsgInfo;
struct JmyEventInfo;
class ConnConfigHandler
{
public:
	static int processConnectResponse(JmyMsgInfo*);
	static int processNewLoginNotify(JmyMsgInfo*);
	static int onConnect(JmyEventInfo*);
	static int onDisconnect(JmyEventInfo*);
	static int onTick(JmyEventInfo*);
	static int onTimer(JmyEventInfo*);

private:
	static char tmp_[MAX_SEND_BUFFER_SIZE];
};
