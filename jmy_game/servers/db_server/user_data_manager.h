#pragma once

#include <string>
#include <unordered_map>
#include <set>
#include "../libjmy/jmy_util.h"

enum { MAX_USER_DATA_COUNT = 200000 };
enum UserState {
	USER_STATE_NONE = 0,
	USER_STATE_ONLINE = 1,
	USER_STATE_OFFLINE = 2,
};

struct UserData {
	int id;
	uint64_t uid; // low 32 bits: database index,  high 32 bits: game id
	std::string account;
	UserState state;
	UserData() : id(0), uid(0), state(USER_STATE_NONE) {}
};

class UserDataManager : public JmySingleton<UserDataManager>
{
public:
	UserDataManager();
	~UserDataManager();

	bool init(int user_count = MAX_USER_DATA_COUNT);
	void clear();
	UserData* getFree(const std::string& account);
	UserData* get(const std::string& account);
	UserData* get(int id);
	bool free(int id);
	bool free(const std::string& account);

private:
	std::unordered_map<std::string, int> account2id_map_;
	UserData** users_data_;
	int user_count_;
	JmyIdGenerator<int> id_gen_;
};

#define USER_MGR (UserDataManager::getInstance())
