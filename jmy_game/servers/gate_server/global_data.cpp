#include "global_data.h"
#include "gate_server.h"
#include "../../proto/src/msgid.pb.h"

GlobalData::GlobalData() : the_game_id_(0)
{
}

GlobalData::~GlobalData()
{
}

bool GlobalData::addLoginClient(int login_id, JmyTcpClient* client)
{
	std::unordered_map<int, JmyTcpClient*>::iterator it = login_clients_.find(login_id);
	if (it != login_clients_.end())
		return false;
	login_clients_.insert(std::make_pair(login_id, client));
	return true;
}

bool GlobalData::removeLoginClient(int login_id)
{
	std::unordered_map<int, JmyTcpClient*>::iterator it = login_clients_.find(login_id);
	if (it == login_clients_.end())
		return false;
	JmyTcpClient* client = it->second;
	login_clients_.erase(login_id);
	GATE_SERVER->removeLoginClient(client);
	return true;
}

bool GlobalData::removeLoginClientByConnId(int conn_id)
{
	int login_id = getLoginIdByConnId(conn_id);
	LogInfo("login_id(%d) get by conn_id(%d)", login_id, conn_id);
	if (!login_id) return false;
	return removeLoginClient(login_id);
}

LoginAgent* GlobalData::newLoginAgent(int login_id, JmyTcpConnectionMgr* conn_mgr, int conn_id)
{
	return login_mgr_.newAgent(login_id, conn_mgr, conn_id);
}

LoginAgent* GlobalData::getLoginAgent(int login_id)
{
	return login_mgr_.getAgent(login_id);
}

LoginAgent* GlobalData::getLoginAgentByConnId(int conn_id)
{
	return login_mgr_.getAgentByConnId(conn_id);
}

bool GlobalData::removeLoginAgent(int login_id)
{
	return login_mgr_.deleteAgent(login_id);
}

bool GlobalData::removeLoginAgentByConnId(int conn_id)
{
	return login_mgr_.deleteAgentByConnId(conn_id);
}

bool GlobalData::removeLoginAgentAndClientByConnId(int conn_id)
{
	int login_id = getLoginIdByConnId(conn_id);
	if (!login_mgr_.deleteAgent(login_id))
		return false;
	return removeLoginClient(login_id);
}

int GlobalData::getLoginIdByConnId(int conn_id)
{
	int login_id = 0;
	if (!login_mgr_.getIdByConnId(conn_id, login_id))
		return 0;
	return login_id;
}

GameAgent* GlobalData::newGameAgent(int game_id, JmyTcpConnectionMgr* conn_mgr, int conn_id)
{
	GameAgent* agent = game_mgr_.getAgentByConnId(conn_id);
	if (agent) {
		LogWarn("already exist game_server connection(conn_id: %d)", conn_id);
		return nullptr;
	}

	if (the_game_id_ > 0) {
		LogError("to game agent(%d) connection is normal, can not allow to other game agent connect", the_game_id_);
		return nullptr;
	}
	agent = game_mgr_.newAgent(game_id, conn_mgr, conn_id);
	if (!agent) {
		LogError("create new game agent with id(%d) failed", game_id);
		return nullptr;
	}

	the_game_id_ = game_id;
	return agent;
}

bool GlobalData::removeGameAgentByConnId(int conn_id)
{
	GameAgent* agent = game_mgr_.getAgentByConnId(conn_id);
	if (!agent) {
		return false;
	}

	int game_id = 0;
	bool b = game_mgr_.getIdByConnId(conn_id, game_id);
	if (!b || (b && game_id != the_game_id_)) {
		LogError("game agent id %d not equal to the game id %d", agent->getConnId(), the_game_id_);
		return false;
	}
	game_mgr_.deleteAgent(the_game_id_);
	the_game_id_ = 0;

	return true;
}

int GlobalData::sendGameMsg(int msg_id, const char* data, unsigned short len)
{
	GameAgent* agent = game_mgr_.getAgent(the_game_id_);
	if (!agent) {
		LogError("cant found game agent(%d) connection", the_game_id_);
		return -1;
	}

	if (agent->sendMsg(msg_id, data, len) < 0) {
		LogError("send message(%d) failed", msg_id);
		return -1;
	}
	return len;
}

int GlobalData::sendGameMsg(int user_id, int msg_id, const char* data, unsigned short len)
{
	GameAgent* agent = game_mgr_.getAgent(the_game_id_);
	if (!agent) {
		LogError("cant found game agent(%d) connection", the_game_id_);
		return -1;
	}

	if (agent->sendMsg(user_id, msg_id, data, len) < 0) {
		LogError("send user(%d) message(%d) failed", user_id, msg_id);
		return -1;
	}
	return len;
}

int send_error(JmyTcpConnection* conn, ProtoErrorType error)
{
	char tmp[64];
	if (!conn) return -1;
	MsgError response;
	response.set_error_code(error);
	response.SerializeToArray(tmp, sizeof(tmp));
	return conn->send(MSGID_ERROR, tmp, response.ByteSize());
}
