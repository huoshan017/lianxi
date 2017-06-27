#pragma once

#include <string>
#include <list>
#include <set>
#include <unordered_map>
#include "../libjmy/jmy_singleton.hpp"
#include "../common/bi_map.h"
#include "global_data.h"
#include "item.h"

struct Player
{
	int user_id;
	uint64_t role_id;
	std::string acount;
	int level;
	ItemMgr items;
	Player() : user_id(0), role_id(0) {}

	bool is_online() { return user_id > 0; }
	int send_gate(int msg_id, const char* data, unsigned short len) {
		if (!user_id) return 0;
		return (SEND_GATE_USER_MSG(user_id, msg_id, data, len) < 0);
	}
};

class PlayerPool
{
public:
	PlayerPool();
	~PlayerPool();

	bool init();
	void clear();

	Player* malloc();
	bool free(Player*);

private:
	void grow();

private:
	std::list<Player*> free_list_;
	std::set<Player*> used_set_;
};

class PlayerManager : public JmySingleton<PlayerManager>
{
public:
	PlayerManager();
	~PlayerManager();

	bool init(int start_id, int max_size);
	void clear();

	Player* malloc(int user_id, uint64_t role_id);
	Player* malloc(uint64_t role_id);
	bool bind_account_role_id(const std::string& account, uint64_t role_id);
	Player* get(int user_id);
	Player* getByRoleId(uint64_t role_id);
	Player* getByAccount(const std::string& account);
	bool free(int role_id);
	bool free(Player* player);
	int getUserIdByRoleId(uint64_t role_id);
	uint64_t getRoleIdByUserId(int user_id);

private:
	int start_player_id_;
	int max_player_size_;
	uint64_t* online_role_ids_;
	BiMap<std::string, uint64_t> account2role_id_;
	std::unordered_map<uint64_t, Player*> all_players_;
	PlayerPool pool_;
	std::string tmp_account_;
};

#define PLAYER_MGR (PlayerManager::getInstance())
