#pragma once

#include "../libjmy/jmy_const.h"
#include "../common/agent.h"
#include <string>
#include <unordered_map>
#include <set>

struct GameAgentData {
};

typedef Agent<GameAgentData, int> GameAgent;
typedef AgentManagerPerf<GameAgentData, int, int, GAME_SERVER_MAX_ID, GAME_SERVER_MIN_ID> GameAgentManager;

class GameHandler
{
public:
	static int onConnect(JmyEventInfo*);
	static int onDisconnect(JmyEventInfo*);
	static int onTick(JmyEventInfo*);

private:
	static char tmp_[JMY_MAX_MSG_SIZE];
};
