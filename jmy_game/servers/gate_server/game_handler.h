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

	static int getGameServerCount() { return (int)game_mgr_.getAgentSize(); }
	static int sendMsg(int user_id, int msg_id, const char* data, unsigned short len);

private:
	static char tmp_[JMY_MAX_MSG_SIZE];
	static GameAgentManager game_mgr_;
	static int the_game_id_;
};

#define SEND_GAME_MSG(user_id, msg_id, data, len) (GameHandler::send(user_id, msg_id, data, len))
