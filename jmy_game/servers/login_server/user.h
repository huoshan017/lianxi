#pragma once

#include <string>
#include <unordered_map>
#include "../libjmy/jmy_singleton.hpp"
#include "const.h"

class JmyTcpConnection;
class JmyTcpConnectionMgr;
class User
{
public:
	User();
	~User();
	void init(int id, JmyTcpConnectionMgr* mgr, const char* account);

	int getId() const { return id_; }
	const char* getAccount() { return account_.c_str(); }
	UserState getState() const { return state_; }
	void setState(UserState state) { state_ = state; }

	int sendMsg(int msgid, const char* data, int len);

private:
	int id_;
	JmyTcpConnectionMgr* mgr_;
	std::string account_;
	UserState state_;
};


class UserManager : public JmySingleton<UserManager>
{
public:
	UserManager();
	~UserManager();
	void clear();

	User* newUser(int id, JmyTcpConnectionMgr* mgr, const char* account);
	User* getUser(const std::string& account);
	User* getUserById(int id);
	bool deleteUser(const std::string& account);
	bool deleteUserById(int id); 
	bool setUserState(const std::string& account, UserState state);
	bool setUserStateById(int id, UserState state);

private:
	std::unordered_map<std::string, int> account2id_;
	std::unordered_map<int, User*> id2user_;
};

#define USER_MGR (UserManager::getInstance())
