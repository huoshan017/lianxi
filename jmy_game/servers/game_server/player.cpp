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
	uid2id_map_.clear();
	account2id_map_.clear();
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
	uid2id_map_.insert(uid, id);
	return p;
}

Player* PlayerManager::get(int id)
{
	int index = id - start_player_id_;
	if (index >= max_player_size_)
		return nullptr;
	if (!player_array_)
		return nullptr;
	Player* p = player_array_[index];
	return p;
}

Player* PlayerManager::getByUid(uint64_t uid)
{
	int user_id = 0;
	bool b = uid2id_map_.find_1(uid, user_id);
	if (!b)
		return nullptr;
	return get(user_id);
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
	int user_id = 0;
	if (!uid2id_map_.find_1(uid, user_id))
		return false;

	return isUsing(user_id);
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
	uid2id_map_.remove_2(id);
	return true;
}

bool PlayerManager::free(Player* p)
{
	return free(p->id);
}

bool PlayerManager::addAccountId(const std::string& account, int user_id)
{
	int id = 0;
	if (!account2id_map_.find_1(account, id))
		return false;
	account2id_map_.insert(account, user_id);
	return true;
}

bool PlayerManager::removeAccountId(const std::string& account)
{
	return account2id_map_.remove_1(account);
}

bool PlayerManager::removeAccountId(int user_id)
{
	return account2id_map_.remove_2(user_id);
}

Player* PlayerManager::getByAccount(const std::string& account)
{
	int user_id = 0;
	if (!account2id_map_.find_1(account, user_id))
		return nullptr;
	return get(user_id);
}
