#pragma once

#include "../libjmy/jmy_singleton.hpp"
#include <set>
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
		return db_accounts_.find(account) != accounts_.end();
	}
	bool insertDBAccount(const std::string& account) {
		if (findDBAccount(account)) return false;
		db_accounts_.insert(account);
		return true;
	}

private:
	std::set<std::string> accounts_;
	std::set<std::string> db_accounts_;
};

#define GLOBAL_DATA (GlobalData::getInstance())
