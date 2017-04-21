#pragma once

#include "../libjmy/jmy_singleton.hpp"
#include "../common/agent.h"

struct GameAgentData {
	int game_id;
};

typedef Agent<GameAgentData, int> GameAgent;
typedef AgentManagerPerf<GameAgentData, int, int, GAME_SERVER_MAX_ID, GAME_SERVER_MIN_ID> GameAgentManager;

class JmyTcpConnectionMgr;
class GameServerManager : public JmySingleton<GameServerManager>
{
public:
	GameServerManager();
	~GameServerManager();

	GameAgent* newAgent(int game_id, JmyTcpConnectionMgr* conn_mgr, int conn_id);
	GameAgent* get(int game_id);
	GameAgent* getByConnId(int conn_id);
	bool remove(int game_id);
	bool removeByConnId(int conn_id);
	int getIdByConnId(int conn_id);

private:
	GameAgentManager game_mgr_;
};

#define GAME_MGR (GameServerManager::getInstance())
