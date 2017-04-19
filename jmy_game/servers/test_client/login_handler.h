#pragma once

#include "../libjmy/jmy_const.h"

struct JmyMsgInfo;
struct JmyEventInfo;
class LoginHandler
{
public:
	static int onConnect(JmyEventInfo*);
	static int onDisconnect(JmyEventInfo*);
	static int onTick(JmyEventInfo*);
	static int processLogin(JmyMsgInfo*);
	static int processSelectedServer(JmyMsgInfo*); 
	static int processDefault(JmyMsgInfo*);

private:
	static char tmp_[JMY_MAX_MSG_SIZE];
};
