#pragma once

#include "../libjmy/jmy_singleton.hpp"
#include <set>
#include <string>

class GlobalData : public JmySingleton<GlobalData>
{
public:
	GlobalData() {}
	~GlobalData() { clear(); }

	void clear() { accounts_.clear(); }

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
			return empty_str_;
		} else {
			return *it;
		}
	}
	bool findAccount(const std::string& account) {
		return accounts_.find(account) != accounts_.end();
	}
	bool removeAccount(const std::string& account) {
		return accounts_.erase(account) > 0;
	}

private:
	std::set<std::string> accounts_;
	std::string empty_str_;
};

#define GLOBAL_DATA (GlobalData::getInstance())
