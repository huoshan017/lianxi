#pragma once

#include "../libjmy/jmy_datatype.h"

struct JmyMsgInfo;
struct JmyEventInfo;
class ConnDBHandler 
{
public:
	static int onConnect(JmyEventInfo*);
	static int onDisconnect(JmyEventInfo*);
	static int onTick(JmyEventInfo*);
	static int processConnectDBResponse(JmyMsgInfo*);
	static int processGetRoleResponse(JmyMsgInfo*);
	static int processCreateRoleResponse(JmyMsgInfo*);
	static int processEnterGameResponse(JmyMsgInfo*);
	static int processDefault(JmyMsgInfo*);

private:
	static char tmp_[JMY_MAX_MSG_SIZE];
};
