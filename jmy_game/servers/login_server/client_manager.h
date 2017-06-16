#pragma once

#include <string>
#include <list>
#include "../libjmy/jmy_singleton.hpp"
#include "../common/agent.h"
#include "../../proto/src/error.pb.h"
#include "../../proto/src/server.pb.h"
#include "config_loader.h"

struct ClientData {
	int gate_id;
};
typedef Agent<ClientData, int> ClientAgent;
typedef AgentManager<std::string, ClientData, int> ClientAgentManager;

class ClientManager : public JmySingleton<ClientManager>
{
public:
	ClientManager();
	~ClientManager();

	ClientAgent* newAgent(const std::string& account, JmyMsgInfo* info);
	ClientAgent* getAgent(const std::string& account);
	ClientAgent* getAgentByConnId(int conn_id);
	bool deleteAgent(const std::string& account);
	bool deleteAgentByConnId(int conn_id);
	bool getAccountByConnId(int conn_id, std::string& account);
	bool isFull() { return ((int)agent_mgr_.getAgentSize() >= SERVER_CONFIG.max_conn); }

private:
	ClientAgentManager agent_mgr_;
};

#define CLIENT_MANAGER (ClientManager::getInstance())
