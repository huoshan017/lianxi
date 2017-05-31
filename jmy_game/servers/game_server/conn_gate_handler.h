#pragma once

#include "../libjmy/jmy_const.h"
#include "gm.h"

struct JmyEventInfo;
struct JmyMsgInfo;
class JmyTcpConnection;
class ConnGateHandler
{
public:
	static int onConnect(JmyEventInfo*);
	static int onDisconnect(JmyEventInfo*);
	static int onTick(JmyEventInfo*);
	static int processConnectGateResponse(JmyMsgInfo*);

	static int processGetRole(JmyMsgInfo*);
	static int processCreateRole(JmyMsgInfo*);

	static int processEnterGame(JmyMsgInfo*);
	static int processLeaveGame(JmyMsgInfo*);
	static int processDefault(JmyMsgInfo*);

private:
	static int processChat(JmyMsgInfo*);
	static int processSetRoleData(JmyMsgInfo*);

private:
	static char tmp_[JMY_MAX_MSG_SIZE];
	static GmManager gm_;
};
