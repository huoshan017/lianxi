#include "client_handler.h"
#include <random>
#include "../libjmy/jmy.h"
#include "../common/util.h"
#include "../../proto/src/server.pb.h"
#include "config_loader.h"
#include "user.h"
#include "gate_server_list.h"

char ClientHandler::tmp_[JMY_MAX_MSG_SIZE];
ClientAgentManager ClientHandler::client_mgr_;

int ClientHandler::onConnect(JmyEventInfo* info)
{
	(void)info;
	int curr_conn = (int)client_mgr_.getAgentSize();
	if (curr_conn < SERVER_CONFIG.max_conn) {
		return 0;
	}
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) {
		LogError("cant get connection by event info(conn_id:%d)", info->conn_id);
		return -1;
	}
	conn->force_close();
	LogWarn("client connection count(%d) is max", curr_conn);
	return 0;
}

int ClientHandler::onDisconnect(JmyEventInfo* info)
{
	(void)info;
	ClientAgent* agent = client_mgr_.getAgentByConnId(info->conn_id);
	if (!agent) {
		LogError("cant get client agent with conn_id(%d)", info->conn_id);
		return -1;
	}
	client_mgr_.deleteAgentByConnId(info->conn_id);
	LogInfo("connection %d ondisconnect", info->conn_id);
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
	conn->send(MSGID_S2C_LOGIN_RESPONSE, tmp_, response.ByteSize());
}

int ClientHandler::processLogin(JmyMsgInfo* info)
{
	MsgC2S_LoginRequest request;
	if (!request.ParseFromArray(info->data, info->len)) {
		send_error(info, PROTO_ERROR_LOGIN_DATA_INVALID);
		LogError("parse MsgC2LLoginRequest failed");
		return -1;
	}

	request.account();
	request.password();

	ClientAgent* user = client_mgr_.newAgent(request.account(), (JmyTcpConnectionMgr*)info->param, info->conn_id);
	if (!user) {
		send_error(info, PROTO_ERROR_LOGIN_REPEATED);
		LogError("create user by account(%s) failed", request.account().c_str());
		return -1;
	}

	user->setState(AGENT_STATE_VERIFIED);

	MsgS2C_LoginResponse response;
	GateServerList::Iterator it = GATE_SERVER_LIST->begin();
	for (; it!=GATE_SERVER_LIST->end(); ++it) {
		MsgServerInfo* si = response.add_servers();
		si->set_name((*it).name());
		si->set_id((*it).id());
		si->set_is_busy((*it).is_busy());
		si->set_is_maintenance((*it).is_maintenance());
	}
	response.SerializeToArray(tmp_, sizeof(tmp_));
	user->sendMsg(MSGID_S2C_LOGIN_RESPONSE, tmp_, response.ByteSize());
	LogInfo("account(%s) login", request.account().c_str());
	return 0;
}

int ClientHandler::processSelectServer(JmyMsgInfo* info)
{
	MsgC2S_SelectServerRequest request;
	if (!request.ParseFromArray(info->data, info->len)) {
		send_error(info, PROTO_ERROR_LOGIN_DATA_INVALID);
		LogError("parse MsgC2LSelectServerRequest failed");
		return -1;
	}

	ClientAgent* user = client_mgr_.getAgentByConnId(info->conn_id);
	if (!user) {
		send_error(info, PROTO_ERROR_LOGIN_ACCOUNT_OR_PASSWORD_INVALID);
		LogError("cant find user by id(%d)", info->conn_id);
		return -1;
	}

	std::string account;
	if (!client_mgr_.getKeyByConnId(info->conn_id, account)) {
		LogError("cant get account by conn_id(%d)", info->conn_id);
		return -1;
	}

	MsgLS2GT_SelectedServerNotify notify;
	notify.set_account(account);
	notify.SerializeToArray(tmp_, sizeof(tmp_));
	user->sendMsg(MSGID_S2C_SELECT_SERVER_RESPONSE, tmp_, notify.ByteSize());
	LogInfo("account(%d) conn_id(%d) select server", account.c_str(), info->conn_id);

	return 0;
}

ClientAgent* ClientHandler::getClientAgent(const std::string& account)
{
	return client_mgr_.getAgent(account);
}
