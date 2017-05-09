#pragma once

#include "../libjmy/jmy_singleton.hpp"
#include <set>
#include <unordered_map>
#include <string>

class GlobalData : public JmySingleton<GlobalData>
{
public:
	GlobalData() {}
	~GlobalData() { clear(); }

	bool init();

	void clear() {
		accounts_.clear();
		db_accounts_.clear();
	}

	void insertAccount(std::string& account) {
		accounts_.insert(std::move(account));
	}
	const std::string& insertAccount(const std::string& account) {
		auto p = accounts_.insert(account);
		return *p.first;
	}
	const std::string& getAccount(const std::string& account) {
		std::set<std::string>::iterator it = accounts_.find(account);
		if (it == accounts_.end()) {
			accounts_.insert(account);
			it = accounts_.find(account);
		}
		return *it;
	}
	bool findAccount(const std::string& account) {
		return accounts_.find(account) != accounts_.end();
	}
	bool removeAccount(const std::string& account) {
		return accounts_.erase(account) > 0;
	}

	bool findDBAccount(const std::string& account) {
		return db_accounts_.find(account) != db_accounts_.end();
	}
	bool insertDBAccount(const std::string& account) {
		if (findDBAccount(account)) return false;
		db_accounts_.insert(account);
		return true;
	}

	void setAccount2UserId(const std::string& account, int user_id) {
		std::unordered_map<std::string, int>::iterator it = userid2accounts_.find(account);
		if (it != userid2accounts_.end()) {
			it->second = user_id;
		} else {
			userid2accounts_.insert(std::make_pair(account, user_id));
		}
	}

	int getUserIdByAccount(const std::string& account) {
		std::unordered_map<std::string, int>::iterator it = userid2accounts_.find(account);
		if (it == userid2accounts_.end()) return 0;
		return it->second;
	}

	bool deleteUserIdByAccount(const std::string& account) {
		std::unordered_map<std::string, int>::iterator it = userid2accounts_.find(account);
		if (it == userid2accounts_.end()) return false;
		userid2accounts_.erase(account);
		return true;
	}

private:
	std::set<std::string> accounts_;
	std::set<std::string> db_accounts_;
	std::unordered_map<std::string, int> userid2accounts_;
};

#define GLOBAL_DATA (GlobalData::getInstance())
