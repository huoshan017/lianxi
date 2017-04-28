#include "user_data_manager.h"
#include "../libjmy/jmy_mem.h"
#include "../common/util.h"
#include <cstring>

UserDataManager::UserDataManager() : user_count_(0)
{
}

UserDataManager::~UserDataManager()
{
}

bool UserDataManager::init(int user_count)
{
	id_gen_.init(user_count, 1);
	std::size_t s = sizeof(UserData*)*user_count;
	users_data_ = (UserData**)jmy_mem_malloc(s);
	std::memset(users_data_, 0, s);
	user_count_ = user_count;
	return true;
}

void UserDataManager::clear()
{
	id_gen_.clear();
	int i = 0;
	for (; i<user_count_; i++) {
		if (users_data_[i]) {
			jmy_mem_free(users_data_[i]);
			users_data_[i] = nullptr;
		}
	}
	jmy_mem_free(users_data_);
	user_count_ = 0;
}

UserData* UserDataManager::getFree(const std::string& account)
{
	if (account2id_map_.find(account) != account2id_map_.end()) {
		LogError("already exist account %s", account.c_str());
		return nullptr;
	}
	int id = id_gen_.get();
	if (id == 0) {
		LogError("get free id failed");
		return nullptr;
	}
	UserData* user = users_data_[id];
	if (!user) {
		user = jmy_mem_malloc<UserData>();
		users_data_[id] = user;
	}
	account2id_map_.insert(std::make_pair(account, id));
	return user;
}

UserData* UserDataManager::get(const std::string& account)
{
	std::unordered_map<std::string, int>::iterator it = account2id_map_.find(account);
	if (it == account2id_map_.end())
		return nullptr;
	return get(it->second);
}

UserData* UserDataManager::get(int id)
{
	if (id > user_count_) {
		return nullptr;
	}
	return users_data_[id];
}

bool UserDataManager::free(const std::string& account)
{
	std::unordered_map<std::string, int>::iterator it = account2id_map_.find(account);
	if (it == account2id_map_.end()) {
		LogError("not found account %s", account.c_str());
		return false;
	}
	int id = it->second;
	return free(id);
}

bool UserDataManager::free(int id)
{
	if (id > user_count_) {
		LogError("id %d overed user count %d", id, user_count_);
		return false;
	}
	if (users_data_[id]) {
		jmy_mem_free(users_data_[id]);
		users_data_[id] = nullptr;
	}
	id_gen_.free(id);
	return true;
}
