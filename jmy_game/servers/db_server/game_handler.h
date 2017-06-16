#pragma once

#include "../libjmy/jmy_const.h"
#include "db_tables_struct.h"

struct JmyMsgInfo;
struct JmyEventInfo;
class GameHandler
{
public:
	static int onConnect(JmyEventInfo*);
	static int onDisconnect(JmyEventInfo*);
	static int onTick(JmyEventInfo*);

	static int processConnectDBRequest(JmyMsgInfo*);
	static int processGetRole(JmyMsgInfo*);
	static int processCreateRole(JmyMsgInfo*);
	static int processDefault(JmyMsgInfo*);

private:
	static int processSetRoleData(JmyMsgInfo*);
	static int processAddItem(JmyMsgInfo*);
	static int processRmItem(JmyMsgInfo*);

private:
	static char tmp_[JMY_MAX_MSG_SIZE];
};
