#include "client_handler.h"
#include "../libjmy/jmy.h"
#include "../common/util.h"
#include "../../proto/src/common.pb.h"
#include "gate_server.h"

char ClientHandler::tmp_[JMY_MAX_MSG_SIZE];
ClientAgentManager ClientHandler::client_mgr_;
std::unordered_map<std::string, std::string>  ClientHandler::account2session_map_;

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
	(void)info;
	return 0;
}

int ClientHandler::onTimer(JmyEventInfo* info)
{
	(void)info;
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
	MsgC2S_EnterGameRequest request;
	if (!request.ParseFromArray(info->data, info->len)) {
		send_error(info, PROTO_ERROR_LOGIN_DATA_INVALID);
		ServerLogError("parse MsgC2S_EnterGameRequest failed");
		return -1;
	}

	// check enter session
	std::unordered_map<std::string, std::string>::iterator it = account2session_map_.find(request.account());
	if (it == account2session_map_.end()) {
		send_error(info, PROTO_ERROR_LOGIN_ACCOUNT_OR_PASSWORD_INVALID);
		ServerLogError("account %s not found", request.account().c_str());
		return -1;
	}

	if (it->second != request.session_code()) {
		send_error(info, PROTO_ERROR_LOGIN_ACCOUNT_OR_PASSWORD_INVALID);
		ServerLogError("account %s enter session %s invalid, valid session: %s",
				request.account().c_str(), request.session_code().c_str(), it->second.c_str());
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

	char* reconn_session = get_session_code(tmp_, RECONN_SESSION_CODE_BUF_LENGTH);
	agent->getData().reconn_session = reconn_session;

	MsgS2C_EnterGameResponse response;
	response.set_reconnect_session(agent->getData().reconn_session.c_str());
	if (!response.SerializeToArray(tmp_, sizeof(tmp_))) {
		ServerLogError("serialize MsgS2C_EnterGameResponse failed");
		return -1;
	}
	if (agent->sendMsg(MSGID_S2C_ENTER_GAME_RESPONSE, tmp_, response.ByteSize()) < 0) {
		ServerLogError("send message MsgS2C_EnterGameResponse failed");
		return -1;
	}

	return info->len;
}

int ClientHandler::processReconnect(JmyMsgInfo* info)
{
	MsgC2S_ReconnectRequest request;
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

	return info->len;
}

int ClientHandler::processDefault(JmyMsgInfo* info)
{
	return info->len;
}
