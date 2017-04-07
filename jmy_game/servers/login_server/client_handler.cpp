#include "client_handler.h"
#include "../libjmy/jmy.h"
#include "../common/util.h"
#include "../../proto/src/server.pb.h"
#include "config_loader.h"
#include "user.h"
#include <random>

char ClientHandler::tmp_[MAX_SEND_BUFFER_SIZE];
ClientAgentManager ClientHandler::client_mgr_;

int ClientHandler::onConnect(JmyEventInfo* info)
{
	(void)info;
	int curr_conn = (int)client_mgr_.getAgentSize();
	if (curr_conn < SERVER_CONFIG_FILE.max_conn) {
		return 0;
	}
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) {
		ServerLogError("cant get connection by event info(conn_id:%d)", info->conn_id);
		return -1;
	}
	conn->force_close();
	ServerLogWarn("client connection count(%d) is max", curr_conn);
	return 0;
}

int ClientHandler::onDisconnect(JmyEventInfo* info)
{
	(void)info;
	ClientAgent* agent = client_mgr_.getAgentByConnId(info->conn_id);
	if (!agent) {
		ServerLogError("cant get client agent with conn_id(%d)", info->conn_id);
		return -1;
	}
	client_mgr_.deleteAgentByConnId(info->conn_id);
	ServerLogInfo("connection %d ondisconnect", info->conn_id);
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

void ClientHandler::send_error(JmyMsgInfo* info, ProtoErrorType error) {
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) return;
	MsgError response;
	response.set_error_code(error);
	conn->send(MSGID_LS2CL_LOGIN_RESPONSE, tmp_, response.ByteSize());
}

int ClientHandler::processLogin(JmyMsgInfo* info)
{
	MsgCL2LS_LoginRequest request;
	if (!request.ParseFromArray(info->data, info->len)) {
		send_error(info, PROTO_ERROR_LOGIN_DATA_INVALID);
		ServerLogError("parse MsgC2LLoginRequest failed");
		return -1;
	}

	request.account();
	request.password();

	ClientAgent* user = client_mgr_.newAgent(request.account(), (JmyTcpConnectionMgr*)info->param, info->session_id);
	if (!user) {
		send_error(info, PROTO_ERROR_LOGIN_REPEATED);
		ServerLogError("create user by account(%s) failed", request.account().c_str());
		return -1;
	}

	user->setState(AGENT_STATE_VERIFIED);

	MsgLS2CL_LoginResponse response;
	response.SerializeToArray(tmp_, sizeof(tmp_));
	user->sendMsg(MSGID_LS2CL_LOGIN_RESPONSE, tmp_, response.ByteSize());
	ServerLogInfo("account(%d) login", request.account().c_str());
	return 0;
}

int ClientHandler::processSelectServer(JmyMsgInfo* info)
{
	MsgCL2LS_SelectServerRequest request;
	if (!request.ParseFromArray(info->data, info->len)) {
		send_error(info, PROTO_ERROR_LOGIN_DATA_INVALID);
		ServerLogError("parse MsgC2LSelectServerRequest failed");
		return -1;
	}

	ClientAgent* user = client_mgr_.getAgentByConnId(info->session_id);
	if (!user) {
		send_error(info, PROTO_ERROR_LOGIN_ACCOUNT_OR_PASSWORD_INVALID);
		ServerLogError("cant find user by id(%d)", info->session_id);
		return -1;
	}

	std::string account;
	if (!client_mgr_.getKeyByConnId(info->session_id, account)) {
		ServerLogError("cant get account by conn_id(%d)", info->session_id);
		return -1;
	}

	MsgLS2GT_SelectedServerNotify notify;
	notify.set_account(account);
	notify.SerializeToArray(tmp_, sizeof(tmp_));
	user->sendMsg(MSGID_LS2CL_SELECT_SERVER_RESPONSE, tmp_, notify.ByteSize());
	ServerLogInfo("user(%d) select server", user->getId());

	return 0;
}

ClientAgent* ClientHandler::getClientAgent(const std::string& account)
{
	return client_mgr_.getAgent(account);
}
