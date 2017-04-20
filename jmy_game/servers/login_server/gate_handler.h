#pragma once

#include "../libjmy/jmy_const.h"
#include "../common/agent.h"
#include "../common/defines.h"
#include <string>
#include <unordered_map>

struct JmyMsgInfo;
struct JmyEventInfo;
class GateHandler
{
public:
	static int init();
	static int processConnectLogin(JmyMsgInfo* info);
	static int processSelectedServerResponse(JmyMsgInfo* info);
	static int onConnect(JmyEventInfo*);
	static int onDisconnect(JmyEventInfo*);
	static int onTick(JmyEventInfo*);

private:
	static char tmp_[JMY_MAX_MSG_SIZE];
};

