#pragma once

#include "../libjmy/jmy_const.h"
#include <unordered_map>

struct JmyEventInfo;
struct JmyMsgInfo;
class JmyTcpConnection;
class ConnGateHandler
{
public:
	static int onConnect(JmyEventInfo*);
	static int onDisconnect(JmyEventInfo*);
	static int onTick(JmyEventInfo*);
	static int onTimer(JmyEventInfo*);
	static int processConnectGateResponse(JmyMsgInfo*);
	static int processEnterGame(JmyMsgInfo*);
	static int processLeaveGame(JmyMsgInfo*);
	static int processDefault(JmyMsgInfo*);

private:
	static char tmp_[JMY_MAX_MSG_SIZE];
	static JmyTcpConnection* gate_conn_;
	static std::unordered_map<std::string, int> account2id_map_;
};
