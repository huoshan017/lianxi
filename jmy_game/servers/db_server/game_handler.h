#pragma once

#include "../libjmy/jmy_const.h"
#include "../common/agent.h"
#include <string>
#include <unordered_map>
#include <set>
#include "user_data_manager.h"
#include "mysql_connector_pool.h"

struct GameAgentData {
};

typedef Agent<GameAgentData, int> GameAgent;
typedef AgentManagerPerf<GameAgentData, int, int, GAME_SERVER_MAX_ID, GAME_SERVER_MIN_ID> GameAgentManager;

class GameHandler
{
public:
	static bool init();
	static void clear();
	static int onConnect(JmyEventInfo*);
	static int onDisconnect(JmyEventInfo*);
	static int onTick(JmyEventInfo*);
	static int onConnectDBRequest(JmyMsgInfo*);
	static int onRequireUserDataRequest(JmyMsgInfo*);

private:
	static GameAgentManager game_mgr_;
	static UserDataManager user_mgr_;
	static MysqlConnectorPool conn_pool_;
	static char tmp_[JMY_MAX_MSG_SIZE];
};
