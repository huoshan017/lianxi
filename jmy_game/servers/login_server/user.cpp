#include "user.h"
#include "../libjmy/jmy.h"
#include "../common/util.h"

/* User */
User::User() : id_(0), mgr_(nullptr), state_(USER_STATE_IDLE)
{
}

User::~User()
{
}

void User::init(int id, JmyTcpConnectionMgr* mgr, const char* account)
{
	id_ = id;
	mgr_ = mgr;
	account_ = account;
	state_ = USER_STATE_IDLE;
}

int User::sendMsg(int msgid, const char* data, int len)
{
	if (state_ != USER_STATE_VERIFIED) return 0;
	JmyTcpConnection* conn = mgr_->get(id_);
	if (!conn) {
		LogError("not found connection with id(%d)", id_);
		return -1;
	}

	if (conn->send(msgid, data, len) < 0) {
		LogError("user(%s) send message failed", account_.c_str());
		return -1;
	}

	return 0;
}

/* UserManager */
UserManager::UserManager()
{
}

UserManager::~UserManager()
{
}

void UserManager::clear()
{
}

User* UserManager::newUser(int id, JmyTcpConnectionMgr* mgr, const char* account)
{
	std::unordered_map<int, User*>::iterator it = id2user_.find(id);
	if (it != id2user_.end()) {
		LogError("already has user(%d), create user failed", id);
		return nullptr;
	}
	std::unordered_map<std::string, int>::iterator iit = account2id_.find(account);
	if (iit != account2id_.end()) {
		LogError("already has account(%s), create user failed", account);
		return nullptr;
	}
	User* user = jmy_mem_malloc<User>();
	user->init(id, mgr, account);
	id2user_.insert(std::make_pair(id, user));
	account2id_.insert(std::make_pair(account, id));
	return user;
}

User* UserManager::getUser(const std::string& account)
{
	std::unordered_map<std::string, int>::iterator it = account2id_.find(account);
	if (it == account2id_.end()) {
		LogError("cant found account(%s)", account.c_str());
		return nullptr;
	}
	return getUserById(it->second);
}

User* UserManager::getUserById(int id)
{
	std::unordered_map<int, User*>::iterator it = id2user_.find(id);
	if (it == id2user_.end()) {
		LogError("cant found user by id(%d)", id);
		return nullptr;
	}
	return it->second;
}

bool UserManager::deleteUser(const std::string& account)
{
	std::unordered_map<std::string, int>::iterator it = account2id_.find(account);
	if (it == account2id_.end()) {
		LogError("cant found account(%s)", account.c_str());
		return false;
	}
	if (!deleteUserById(it->second)) {
		return false;
	}
	account2id_.erase(it);
	return true;
}

bool UserManager::deleteUserById(int id)
{
	std::unordered_map<int, User*>::iterator it = id2user_.find(id);
	if (it == id2user_.end()) {
		LogError("not found user(%d)", id);
		return false;
	}
	jmy_mem_free<User>(it->second);
	id2user_.erase(id);
	return true;
}

bool UserManager::setUserState(const std::string& account, UserState state)
{
	std::unordered_map<std::string, int>::iterator it = account2id_.find(account);
	if (it == account2id_.end()) {
		LogError("not found account(%s)", account.c_str());
		return false;
	}
	return setUserStateById(it->second, state);
}

bool UserManager::setUserStateById(int id, UserState state)
{
	std::unordered_map<int, User*>::iterator it = id2user_.find(id);
	if (it == id2user_.end()) {
		LogError("not found user(%d)", id);
		return false;
	}
	if (!it->second) {
		LogError("null user pointer");
		return false;
	}
	it->second->setState(state);
	return true;
}
