#pragma once

#include "../libjmy/jmy_const.h"
#include "../common/agent.h"
#include "../common/defines.h"

struct JmyMsgInfo;
struct JmyEventInfo;
class GameHandler
{
public:
	static int onConnect(JmyEventInfo*);
	static int onDisconnect(JmyEventInfo*);
	static int onTick(JmyEventInfo*);
	static int processConnectGateRequest(JmyMsgInfo*);
	static int processEnterGameResponse(JmyMsgInfo*);
	static int processLeaveGameResponse(JmyMsgInfo*);
	static int processDefault(JmyMsgInfo*);

private:
	static char tmp_[JMY_MAX_MSG_SIZE];
};
