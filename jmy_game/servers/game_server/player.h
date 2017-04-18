#pragma once

#include <string>
#include <unordered_map>
#include "../libjmy/jmy_singleton.hpp"

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
	void clear();

	Player* malloc(int id, uint64_t uid);
	Player* get(int id);
	Player* getByUid(uint64_t uid);
	bool isUsing(int id);
	bool isUsing(uint64_t uid);
	bool free(int id);
	bool free(Player* player);

private:
	Player** player_array_;
	bool* using_array_;
	int start_player_id_;
	int max_player_size_;
	std::unordered_map<uint64_t, int> uid2id_map_;
};

#define PLAYER_MGR (PlayerManager::getInstance())
