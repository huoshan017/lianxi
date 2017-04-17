#pragma once

#include "../libjmy/jmy_const.h"
#include "../common/agent.h"
#include <set>
#include <string>
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

	// callback functions
	static int getPlayerInfoCallback(MysqlConnector::Result& res, void* param, long param_l);
	static int insertPlayerInfoCallback(MysqlConnector::Result& res, void* param, long param_l);
	static int getPlayerUidCallback(MysqlConnector::Result& res, void* param, long param_l);

private:
	static GameAgentManager game_mgr_;
	static UserDataManager user_mgr_;
	static MysqlConnectorPool conn_pool_;
	static char tmp_[JMY_MAX_MSG_SIZE];

	static std::set<std::string> accounts_set_;
};
