#include "client_manager.h"
#include "../common/util.h"

ClientManager::ClientManager()
{
}

ClientManager::~ClientManager()
{
}

ClientAgent* ClientManager::newAgent(const std::string& account, JmyMsgInfo* info)
{
	return agent_mgr_.newAgent(account, (JmyTcpConnectionMgr*)info->param, info->conn_id);
}

ClientAgent* ClientManager::getAgent(const std::string& account)
{
	return agent_mgr_.getAgent(account);
}

ClientAgent* ClientManager::getAgentByConnId(int conn_id)
{
	return agent_mgr_.getAgentByConnId(conn_id);
}

bool ClientManager::deleteAgent(const std::string& account)
{
	return agent_mgr_.deleteAgent(account);
}

bool ClientManager::deleteAgentByConnId(int conn_id)
{
	return agent_mgr_.deleteAgentByConnId(conn_id);
}

bool ClientManager::getAccountByConnId(int conn_id, std::string& account)
{
	return agent_mgr_.getKeyByConnId(conn_id, account);
}
