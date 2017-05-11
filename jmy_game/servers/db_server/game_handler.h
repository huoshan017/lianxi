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
	static int processRequireUserDataRequest(JmyMsgInfo*);

private:
	static char tmp_[JMY_MAX_MSG_SIZE];
	static t_player tmp_player_;
};
