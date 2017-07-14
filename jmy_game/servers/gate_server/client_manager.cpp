#include "client_manager.h"
#include "config_loader.h"

ClientManager::ClientManager()
{
}

ClientManager::~ClientManager()
{
}

bool ClientManager::init()
{
	if (!client_array_.init(CONFIG_FILE.max_conn)) {
		return false;
	}
	return true;
}

void ClientManager::clear()
{
	account_session_map_.clear();
	connid_id_bimap_.clear();
	account_id_bimap_.clear();
	client_array_.clear();
}

bool ClientManager::newClientSession(const std::string& account, const std::string& session_code)
{
	ClientInfo* info = newClient(account);
	if (!info) return false;
	info->enter_session = session_code;
	return true;
}

bool ClientManager::newAccountSession(const std::string& account, const std::string& session)
{
	std::unordered_map<std::string, SessionInfo>::iterator it = account_session_map_.find(account);
	if (it == account_session_map_.end()) {
		SessionInfo si;
		si.session = session;
		si.get_session_time = std::time(0);
		account_session_map_.insert(std::make_pair(account, si));
	} else {
		it->second.session = session;
		it->second.get_session_time = std::time(0);
	}
	return true;
}

bool ClientManager::checkSessionByAccount(const std::string& account, const std::string& session)
{
	std::unordered_map<std::string, SessionInfo>::iterator it = account_session_map_.find(account);
	if (it == account_session_map_.end())
		return false;
	if (it->second.session != session)
		return false;
	if (std::time(0) - it->second.get_session_time > 10) {
		account_session_map_.erase(it);
		return false;
	}
	account_session_map_.erase(it);
	return true;
}

ClientInfo* ClientManager::newClient(const std::string& account)
{
	int id = 0;
	ClientInfo* info = nullptr;
	if (account_id_bimap_.find_1(account, id)) {
		info = client_array_.get(id);
		if (!info) {
			LogError("cant found ClientInfo by account:%s id:%d", account.c_str(), id);
			return nullptr;
		}
		info->force_close();
	} else {
		info = client_array_.getFree();
		if (!info) {
			LogError("cant get free ClientInfo for account(%s)", account.c_str());
			return nullptr;
		}
		account_id_bimap_.insert(account, info->id);
	}
	info->account = account;
	return info;
}

ClientInfo* ClientManager::getClientInfo(int id)
{
	return client_array_.get(id);
}

ClientInfo* ClientManager::getClientInfoByAccount(const std::string& account)
{
	int id = 0;
	bool b = account_id_bimap_.find_1(account, id);
	if (!b) return nullptr;
	return client_array_.get(id);
}

ClientInfo* ClientManager::getClientInfoByConnId(int conn_id)
{
	int id = 0;
	bool b = connid_id_bimap_.find_1(conn_id, id);
	if (!b) return nullptr;
	return client_array_.get(id);
}

bool ClientManager::insertConnIdId(int conn_id, int id)
{
	return connid_id_bimap_.insert(conn_id, id);
}

int ClientManager::getIdByConnId(int conn_id)
{
	int id = 0;
	if (!connid_id_bimap_.find_1(conn_id, id))
		return 0;
	return id;
}

bool ClientManager::removeByConnId(int conn_id)
{
	int id = 0;
	if (!connid_id_bimap_.find_1(conn_id, id)) {
		LogError("cant found id by conn_id(%d)", conn_id);
		return false;
	}
	if (!client_array_.free(id)) {
		LogError("cant free ClientInfo by id(%d)", id);
		return false;
	}
	if (!account_id_bimap_.remove_2(id)) {
		LogError("cant remove account with id(%d)", id);
		return false;
	}
	return connid_id_bimap_.remove_1(conn_id);
}
