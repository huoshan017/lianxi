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
	bool findAccount(const std::string& account) {
		return accounts_.find(account) != accounts_.end();
	}
	bool removeAccount(const std::string& account) {
		return accounts_.erase(account) > 0;
	}

private:
	std::set<std::string> accounts_;
};

#define GLOBAL_DATA (GlobalData::getInstance())
