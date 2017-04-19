#pragma once
#include "../libjmy/jmy_singleton.hpp"
#include "../common/agent.h"
#include <unordered_map>

// login
struct LoginAgentData {
};
typedef Agent<LoginAgentData, int> LoginAgent;
typedef AgentManagerPerf<LoginAgentData, int, int, LOGIN_SERVER_MAX_ID, LOGIN_SERVER_MIN_ID> LoginAgentManager;

// game
struct GameData {
};
typedef Agent<GameData, int> GameAgent;
typedef AgentManagerPerf<GameData, int, int, GAME_SERVER_MAX_ID, GAME_SERVER_MIN_ID> GameAgentManager;

class JmyTcpClient;
class JmyTcpConnectionMgr;
class JmyTcpConnection;
class GlobalData : public JmySingleton<GlobalData>
{
public:
	GlobalData();
	~GlobalData();

	bool addLoginClient(int login_id, JmyTcpClient* client);
	bool removeLoginClient(int login_id);
	bool removeLoginClientByConnId(int conn_id);

	LoginAgent* newLoginAgent(int login_id, JmyTcpConnectionMgr* conn_mgr, int conn_id);
	LoginAgent* getLoginAgent(int login_id);
	LoginAgent* getLoginAgentByConnId(int conn_id);
	bool removeLoginAgent(int login_id);
	bool removeLoginAgentByConnId(int conn_id);
	bool removeLoginAgentAndClientByConnId(int conn_id);
	int getLoginIdByConnId(int conn_id);

	GameAgent* newGameAgent(int game_id, JmyTcpConnectionMgr* conn_mgr, int conn_id);
	bool removeGameAgentByConnId(int conn_id);
	int sendGameMsg(int user_id, int msg_id, const char* data, unsigned short len);
	int getGameServerId() { return the_game_id_; }

private:
	std::unordered_map<int, JmyTcpClient*> login_clients_;
	LoginAgentManager login_mgr_;
	GameAgentManager game_mgr_;
	int the_game_id_;
};

#define GLOBAL_DATA									(GlobalData::getInstance())
#define SEND_GAME_MSG(user_id, msg_id, data, len)	(GLOBAL_DATA->sendGameMsg(user_id, msg_id, data, len))
#define GAME_SERVER_ID								(GLOBAL_DATA->getGameServerId())
