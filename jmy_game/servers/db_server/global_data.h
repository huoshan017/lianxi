#pragma once

#include "../libjmy/jmy_singleton.hpp"
#include <set>
#include <unordered_map>
#include <string>
#include "db_tables_struct.h"

class GlobalData : public JmySingleton<GlobalData>
{
public:
	GlobalData() {}
	~GlobalData() { clear(); }

	bool init();

	void clear() {
		db_accounts_.clear();
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

	void run() {
		tables_mgr_.update();
	}

	jmy_game_tables_manager& getTablesMgr() { return tables_mgr_; }

private:
	std::set<std::string> db_accounts_;
	std::unordered_map<std::string, int> userid2accounts_;
	jmy_game_tables_manager tables_mgr_;
};

#define GLOBAL_DATA (GlobalData::getInstance())
#define TABLES_MGR (GlobalData::getInstance()->getTablesMgr())
