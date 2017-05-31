#include "player.h"
#include "../libjmy/jmy_mem.h"
#include <cstring>

enum { ONCE_MALLOC_PLAYER_OBJ_COUNT = 1000 };

PlayerPool::PlayerPool()
{
}

PlayerPool::~PlayerPool()
{
}

void PlayerPool::grow()
{
	int i = 0;
	for (; i<ONCE_MALLOC_PLAYER_OBJ_COUNT; ++i) {
		Player* p = jmy_mem_malloc<Player>();
		if (p) free_list_.push_back(p);
	}
}

bool PlayerPool::init()
{
	if (free_list_.size() == 0)
		grow();
	return true;
}

void PlayerPool::clear()
{
	std::list<Player*>::iterator it = free_list_.begin();
	for (; it!=free_list_.end(); ++it) {
		if (*it) {
			jmy_mem_free(*it);
		}
	}
	free_list_.clear();

	std::set<Player*>::iterator sit = used_set_.begin();
	for (; sit!=used_set_.end(); ++sit) {
		if (*sit) {
			jmy_mem_free(*sit);
		}
	}
	used_set_.clear();
}

Player* PlayerPool::malloc()
{
	if (free_list_.size() < 100) {
		grow();
	}
	Player* p = free_list_.front();
	used_set_.insert(p);
	free_list_.pop_front();
	return p;
}

bool PlayerPool::free(Player* p)
{
	if (used_set_.find(p) == used_set_.end())
		return false;

	used_set_.erase(p);
	free_list_.push_front(p);
	return true;
}

PlayerManager::PlayerManager() : start_player_id_(0), max_player_size_(0), online_role_ids_(nullptr)
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

	if (!online_role_ids_) {
		online_role_ids_ = (uint64_t*)jmy_mem_malloc(sizeof(uint64_t)*max_size);
		std::memset(online_role_ids_, 0, sizeof(uint64_t)*max_size);
		start_player_id_ = start_id;
		max_player_size_ = max_size;
	}

	return true;
}

void PlayerManager::clear()
{
	if (!online_role_ids_)
		return;

	jmy_mem_free(online_role_ids_);
	std::unordered_map<uint64_t, Player*>::iterator it = all_players_.begin();
	for (; it!=all_players_.end(); ++it) {
		if (it->second) {
			jmy_mem_free(it->second);
		}
	}
	all_players_.clear();
	start_player_id_ = 0;
	max_player_size_ = 0;
}

Player* PlayerManager::malloc(int user_id, uint64_t role_id)
{
	int index = user_id - start_player_id_;
	if (!online_role_ids_ || (index<0) || (index>=max_player_size_))
		return nullptr;
	online_role_ids_[index] = role_id;
	Player* p = pool_.malloc();
	p->user_id = user_id;
	p->role_id = role_id;
	all_players_.insert(std::make_pair(role_id, p));
	return p;
}

Player* PlayerManager::malloc(uint64_t role_id)
{
	Player* p = pool_.malloc();
	p->role_id = role_id;
	all_players_.insert(std::make_pair(role_id, p));
	return p;
}

bool PlayerManager::bind_account_role_id(const std::string& account, uint64_t role_id)
{
	if (all_players_.find(role_id) == all_players_.end())
		return false;
	account2role_id_.insert(account, role_id);
	return true;
}

Player* PlayerManager::get(int id)
{
	int index = id - start_player_id_;
	if (index < 0 || index >= max_player_size_)
		return nullptr;
	if (!online_role_ids_)
		return nullptr;
	uint64_t role_id = online_role_ids_[index];
	return getByRoleId(role_id);
}

Player* PlayerManager::getByRoleId(uint64_t role_id)
{
	std::unordered_map<uint64_t, Player*>::iterator it = all_players_.find(role_id);
	if (it == all_players_.end())
		return nullptr;
	return it->second;
}

Player* PlayerManager::getByAccount(const std::string& account)
{
	uint64_t role_id = 0;
	if (!account2role_id_.find_1(account, role_id))
		return nullptr;
	return getByRoleId(role_id);
}

bool PlayerManager::free(int user_id)
{
	int index = user_id - start_player_id_;
	if (index<0 || index>=max_player_size_)
		return false;

	uint64_t role_id = online_role_ids_[index];
	std::unordered_map<uint64_t, Player*>::iterator it = all_players_.find(role_id);
	if (it == all_players_.end())
		return false;

	if (!pool_.free(it->second))
		return false;

	account2role_id_.remove_2(role_id);

	online_role_ids_[index] = 0;
	all_players_.erase(it);

	return true;
}

bool PlayerManager::free(Player* p)
{
	int user_id = p->user_id;
	int index = user_id - start_player_id_;
	if (index < 0 || index >= max_player_size_)
		return false;

	uint64_t role_id = p->role_id;

	if (!pool_.free(p))
		return false;

	account2role_id_.remove_2(role_id);
	
	all_players_.erase(role_id);
	online_role_ids_[index] = 0;

	return true;
}

int PlayerManager::getUserIdByRoleId(uint64_t role_id)
{
	std::unordered_map<uint64_t, Player*>::iterator it = all_players_.find(role_id);
	if (it == all_players_.end())
		return 0;
	if (!it->second)
		return 0;
	return it->second->user_id;
}

uint64_t PlayerManager::getRoleIdByUserId(int user_id)
{
	int index = user_id - start_player_id_;
	if (index<0 || index>=max_player_size_)
		return 0;

	uint64_t role_id = online_role_ids_[index];
	return role_id;
}
