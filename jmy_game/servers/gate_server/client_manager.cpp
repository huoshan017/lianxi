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
	connid_id_bimap_.clear();
	account_id_bimap_.clear();
	client_array_.clear();
}

bool ClientManager::newClientSession(const std::string& account, const std::string& session_code)
{
	int id = 0;
	if (account_id_bimap_.find_1(account, id)) {
		ClientInfo* info = client_array_.get(id);
		if (!info) {
			LogError("cant found ClientInfo by account:%s id:%d", account.c_str(), id);
			return false;
		}
		info->account = account;
		info->enter_session = session_code;
	} else {
		ClientInfo* info = client_array_.getFree();
		if (!info) {
			LogError("cant get free ClientInfo for account(%s) to hold enter_session(%s)", account.c_str(), session_code.c_str());
			return false;
		}
		account_id_bimap_.insert(account, info->id);
	}
	return true;
}

ClientInfo* ClientManager::getClientInfo(int user_id)
{
	return client_array_.get(user_id);
}

ClientInfo* ClientManager::getClientInfoByAccount(const std::string& account)
{
	int id = 0;
	bool b = account_id_bimap_.find_1(account, id);
	if (!b) {
		LogError("account(%s) not found in account_id_map_", account.c_str());
		return nullptr;
	}
	return client_array_.get(id);
}

ClientInfo* ClientManager::getClientInfoByConnId(int conn_id)
{
	int id = 0;
	bool b = connid_id_bimap_.find_1(conn_id, id);
	if (!b) {
		LogError("conn_id(%d) not found in connid_id_map_", conn_id);
		return nullptr;
	}

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
	return connid_id_bimap_.remove_1(conn_id);
}
