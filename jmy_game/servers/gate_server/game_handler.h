#pragma once

#include "../libjmy/jmy_const.h"
#include "../common/agent.h"
#include "../common/defines.h"

struct GameData {
};
typedef Agent<GameData, int> GameAgent;
typedef AgentManagerPerf<GameData, int, int, GAME_SERVER_MAX_ID, GAME_SERVER_MIN_ID> GameAgentManager;

struct JmyMsgInfo;
struct JmyEventInfo;
class GameHandler
{
public:
	static int processConnectGateRequest(JmyMsgInfo*);
	static int onConnect(JmyEventInfo*);
	static int onDisconnect(JmyEventInfo*);
	static int onTick(JmyEventInfo*);
	static int onTimer(JmyEventInfo*);

private:
	static char tmp_[JMY_MAX_MSG_SIZE];
	static GameAgentManager game_mgr_;
};
