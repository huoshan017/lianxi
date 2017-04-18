#include "player.h"
#include "../libjmy/jmy_mem.h"
#include <cstring>

PlayerManager::PlayerManager() : player_array_(nullptr), start_player_id_(0), max_player_size_(0)
{
}

PlayerManager::~PlayerManager()
{
	clear();
}

bool PlayerManager::init(int start_id, int max_size)
{
	if (start_id <= 0 || max_size <= 0)
		return false;

	if (!player_array_) {
		player_array_ = (Player**)jmy_mem_malloc(sizeof(Player*)*max_size);
		std::memset(player_array_, 0, sizeof(Player*)*max_size);
		using_array_ = (bool*)jmy_mem_malloc(sizeof(bool)*max_size);
		std::memset(using_array_, 0, sizeof(bool)*max_size);
	}

	start_player_id_ = start_id;
	max_player_size_ = max_size;

	return true;
}

void PlayerManager::clear()
{
	if (!player_array_)
		return;

	int i = 0;
	for (; i<max_player_size_; ++i) {
		if (player_array_[i]) {
			jmy_mem_free(player_array_[i]);
		}
	}
	jmy_mem_free(player_array_);
	jmy_mem_free(using_array_);
	start_player_id_ = 0;
	max_player_size_ = 0;
}

Player* PlayerManager::malloc(int id, uint64_t uid)
{
	int index = id - start_player_id_;
	if (!player_array_ || (index>=max_player_size_))
		return nullptr;
	Player* p = player_array_[index];
	if (p) return p;
	player_array_[index] = jmy_mem_malloc<Player>();
	p = player_array_[index];
	p->id = id;
	p->uid = uid;
	using_array_[index] = true;
	uid2id_map_.insert(std::make_pair(uid, id));
	return p;
}

Player* PlayerManager::get(int id)
{
	if (id >= max_player_size_ + start_player_id_)
		return nullptr;
	if (!player_array_)
		return nullptr;
	Player* p = player_array_[id-start_player_id_];
	return p;
}

Player* PlayerManager::getByUid(uint64_t uid)
{
	std::unordered_map<uint64_t, int>::iterator it = uid2id_map_.find(uid);
	if (it == uid2id_map_.end())
		return nullptr;
	return get(it->second);
}

bool PlayerManager::isUsing(int id)
{
	int index = id - start_player_id_;
	if (!player_array_ || index>=max_player_size_)
		return false;
	return using_array_[index];
}

bool PlayerManager::isUsing(uint64_t uid)
{
	std::unordered_map<uint64_t, int>::iterator it = uid2id_map_.find(uid);
	if (it == uid2id_map_.end())
		return false;

	return isUsing(it->second);
}

bool PlayerManager::free(int id)
{
	int index = id - start_player_id_;
	if (index >= max_player_size_)
		return false;
	bool u = using_array_[index];
	if (u) {
		using_array_[index] = false;
	}
	return true;
}

bool PlayerManager::free(Player* p)
{
	return free(p->id);
}
