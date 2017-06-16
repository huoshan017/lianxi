#pragma once

#include "../libjmy/jmy_singleton.hpp"
#include "../common/agent.h"
#include "../common/bi_map.h"
#include "client_array.h"

struct ClientData {
	std::string enter_session;
	std::string reconn_session;
};
typedef Agent<ClientData, int> ClientAgent;
typedef AgentManager<std::string, ClientData, int> ClientAgentManager;

class ClientManager : public JmySingleton<ClientManager>
{
public:
	ClientManager();
	~ClientManager();

	bool init();
	void clear();

	bool newClientSession(const std::string& account, const std::string& session_code);
	bool insertConnIdId(int conn_id, int id);
	int getIdByConnId(int conn_id);
	bool removeByConnId(int conn_id);

	int getClientStartId() const { return client_array_.getStartId(); }
	int getUsedSize() const { return client_array_.getUsedSize(); }
	ClientInfo* getClientInfo(int user_id);
	ClientInfo* getClientInfoByAccount(const std::string& account);
	ClientInfo* getClientInfoByConnId(int conn_id);
	
private:
	BiMap<std::string, int> account_id_bimap_;
	BiMap<int, int> connid_id_bimap_;
	ClientArray client_array_;
};

#define CLIENT_MANAGER (ClientManager::getInstance())
