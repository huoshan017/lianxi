#pragma once

#include "../libjmy/jmy_const.h"

struct JmyEventInfo;
class ConnConfigEventHandler
{
public:
	static int onConnect(JmyEventInfo*);
	static int onDisconnect(JmyEventInfo*);
	static int onTick(JmyEventInfo*);
	static int onTimer(JmyEventInfo*);

private:
	static char tmp_[MAX_SEND_BUFFER_SIZE];
};
