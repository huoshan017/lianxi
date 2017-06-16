#include "game_server_manager.h"

GameServerManager::GameServerManager()
{
}

GameServerManager::~GameServerManager()
{
}

GameAgent* GameServerManager::newAgent(int game_id, JmyTcpConnectionMgr* conn_mgr, int conn_id)
{
	return game_mgr_.newAgent(game_id, conn_mgr, conn_id);
}

GameAgent* GameServerManager::get(int game_id)
{
	return game_mgr_.getAgent(game_id);
}

GameAgent* GameServerManager::getByConnId(int conn_id)
{
	return game_mgr_.getAgentByConnId(conn_id);
}

bool GameServerManager::remove(int game_id)
{
	return game_mgr_.deleteAgent(game_id);
}

bool GameServerManager::removeByConnId(int conn_id)
{
	return game_mgr_.deleteAgentByConnId(conn_id);
}

int GameServerManager::getIdByConnId(int conn_id)
{
	int id = 0;
	if (!game_mgr_.getIdByConnId(conn_id, id))
		return 0;
	return id;
}
