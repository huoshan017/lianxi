#pragma once

#include <string>
#include <unordered_map>
#include "../libjmy/jmy_singleton.hpp"
#include "../common/bi_map.h"

struct Player
{
	int id;
	std::string acount;
	uint64_t uid;
	Player() {}
};

class PlayerManager : public JmySingleton<PlayerManager>
{
public:
	PlayerManager();
	~PlayerManager();

	bool init(int start_id, int max_size);
	bool isInited() const { return player_array_!=nullptr; }
	void clear();

	Player* malloc(int user_id, uint64_t unique_id);
	Player* get(int user_id);
	Player* getByUid(uint64_t unique_id);
	bool isUsing(int user_id);
	bool isUsing(uint64_t unique_id);
	bool free(int unique_id);
	bool free(Player* player);
	bool addAccountId(const std::string& account, int user_id);
	bool removeAccountId(const std::string& account);
	bool removeAccountId(int user_id);
	Player* getByAccount(const std::string& account);

private:
	Player** player_array_;
	bool* using_array_;
	int start_player_id_;
	int max_player_size_;
	BiMap<uint64_t, int> uid2id_map_;
	BiMap<std::string, int> account2id_map_;
};

#define PLAYER_MGR (PlayerManager::getInstance())
