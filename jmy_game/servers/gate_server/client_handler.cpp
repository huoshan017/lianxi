#include "client_handler.h"
#include "../libjmy/jmy.h"
#include "../common/util.h"
#include "../../proto/src/common.pb.h"
#include "gate_server.h"

char ClientHandler::tmp_[MAX_SEND_BUFFER_SIZE];
ClientAgentManager ClientHandler::client_mgr_;
std::unordered_map<std::string, std::string>  ClientHandler::account2session_map_;

void ClientHandler::send_error(JmyMsgInfo* info, ProtoErrorType error)
{
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) return;
	MsgError response;
	response.set_error_code(error);
	response.SerializeToArray(tmp_, sizeof(tmp_));
	conn->send(MSGID_ERROR, tmp_, response.ByteSize());
}

int ClientHandler::processEnterGame(JmyMsgInfo* info)
{
	MsgCL2GT_EnterGameRequest request;
	if (!request.ParseFromArray(info->data, info->len)) {
		send_error(info, PROTO_ERROR_LOGIN_DATA_INVALID);
		return -1;
	}

	if (!client_mgr_.getAgent(request.account())) {
		send_error(info, PROTO_ERROR_ENTER_GAME_REPEATED);
		ServerLogError("account(%s) already entered game", request.account().c_str());
		return -1;
	}

	ClientAgent* agent = client_mgr_.newAgent(request.account(), (JmyTcpConnectionMgr*)info->param, info->session_id);
	if (!agent) {
		send_error(info, PROTO_ERROR_ENTER_GAME_FAILED);
		ServerLogError("create new client agent with id(%d) account(%s) failed", info->session_id, request.account().c_str());
		return -1;
	}

	return 0;
}

int ClientHandler::processReconnect(JmyMsgInfo* info)
{
	MsgCL2GT_ReconnectRequest request;
	if (!request.ParseFromArray(info->data, info->len)) {
		ServerLogError("parse MsgCL2GT_ReconnectRequest failed");
		return -1;
	}

	ClientAgent* agent = client_mgr_.getAgent(request.account());
	if (!agent) {
		ServerLogError("not found account %s", request.account().c_str());
		return -1;
	}

	// disconnect previous connection
	agent->close();

	return 0;
}

int ClientHandler::onConnect(JmyEventInfo* info)
{
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) return -1;
	if (GATE_SERVER->checkClientMaxCount((int)client_mgr_.getAgentSize())) {
		conn->close();
		return -1;
	}
	ServerLogInfo("new client connection(%d)", info->conn_id);
	return 0;
}

int ClientHandler::onDisconnect(JmyEventInfo* info)
{
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) return -1;

	ServerLogInfo("client connection(%d) disconnect", info->conn_id);
	return 0;
}

int ClientHandler::onTick(JmyEventInfo* info)
{
	return 0;
}

int ClientHandler::onTimer(JmyEventInfo* info)
{
	return 0;
}

bool ClientHandler::newClientSession(const std::string& account, const std::string& session_code)
{
	if (client_mgr_.getAgent(account)) {
		ServerLogError("already exist account(%s)", account.c_str());
		return false;
	}
	if (account2session_map_.find(account) != account2session_map_.end())
		account2session_map_[account] = session_code;
	else
		account2session_map_.insert(std::make_pair(account, session_code));
	return true;
}
