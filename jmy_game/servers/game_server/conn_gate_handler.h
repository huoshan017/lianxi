#pragma once

#include "../libjmy/jmy_const.h"

struct JmyEventInfo;
struct JmyMsgInfo;
class ConnGateHandler
{
public:
	static int onConnect(JmyEventInfo*);
	static int onDisconnect(JmyEventInfo*);
	static int onTick(JmyEventInfo*);
	static int onTimer(JmyEventInfo*);
	static int processEnterGame(JmyMsgInfo*);
	static int processLeaveGame(JmyMsgInfo*);
	static int processDefault(JmyMsgInfo*);

private:
	static char tmp_[JMY_MAX_MSG_SIZE];
};
