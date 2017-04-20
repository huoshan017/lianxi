#pragma once

#include "../libjmy/jmy_const.h"

struct JmyMsgInfo;
struct JmyEventInfo;
class ConnConfigHandler
{
public:
	static int processConnectConfigResponse(JmyMsgInfo*);
	static int processNewLoginNotify(JmyMsgInfo*);
	static int processRemoveLoginNotify(JmyMsgInfo*);
	static int onConnect(JmyEventInfo*);
	static int onDisconnect(JmyEventInfo*);
	static int onTick(JmyEventInfo*);

private:
	static char tmp_[JMY_MAX_MSG_SIZE];
};
