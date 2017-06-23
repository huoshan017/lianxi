#include "client_handler.h"
#include "../libjmy/jmy.h"
#include "../common/util.h"
#include "../../proto/src/server.pb.h"
#include "../../proto/src/common.pb.h"
#include <random>
#include "config_loader.h"
#include "user.h"
#include "gate_server_list.h"
#include "client_manager.h"

char ClientHandler::tmp_[JMY_MAX_MSG_SIZE];

int ClientHandler::send_error(JmyMsgInfo* info, ProtoErrorType error) {
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) return -1;
	MsgError response;
	response.set_error_code(error);
	return conn->send(MSGID_S2C_LOGIN_RESPONSE, tmp_, response.ByteSize());
}

int ClientHandler::onConnect(JmyEventInfo* info)
{
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) {
		LogError("cant get connection by event info(conn_id:%d)", info->conn_id);
		return -1;
	}
	if (!CLIENT_MANAGER->isFull()) {
		LogInfo("client(conn_id:%d) onConnect", info->conn_id);
		return 0;
	}
	conn->force_close();
	LogWarn("client connection count is max(%d)", SERVER_CONFIG.max_conn);
	return 0;
}

int ClientHandler::onDisconnect(JmyEventInfo* info)
{
	ClientAgent* agent = CLIENT_MANAGER->getAgentByConnId(info->conn_id);
	if (!agent) {
		LogError("cant get client agent with conn_id(%d)", info->conn_id);
		return -1;
	}
	CLIENT_MANAGER->deleteAgentByConnId(info->conn_id);
	LogInfo("connection %d onDisconnect", info->conn_id);
	return 0;
}

int ClientHandler::onTick(JmyEventInfo* info)
{
	(void)info;
	return 0;
}

int ClientHandler::processLogin(JmyMsgInfo* info)
{
	MsgC2S_LoginRequest request;
	if (!request.ParseFromArray(info->data, info->len)) {
		send_error(info, PROTO_ERROR_LOGIN_DATA_INVALID);
		LogError("parse MsgC2S_LoginRequest failed");
		return -1;
	}

	// check account
	ClientAgent* agent = CLIENT_MANAGER->getAgent(request.account());
	if (agent) {
		agent->force_close();
		CLIENT_MANAGER->deleteAgent(request.account());
		LogWarn("already exist account(%s), kick it", request.account().c_str());
	}

	agent = CLIENT_MANAGER->newAgent(request.account(), info);
	if (!agent) {
		send_error(info, PROTO_ERROR_LOGIN_REPEATED);
		LogError("create user by account(%s) failed", request.account().c_str());
		return -1;
	}
	agent->setState(AGENT_STATE_VERIFIED);

	MsgS2C_LoginResponse response;
	GateServerList::Iterator it = GATE_SERVER_LIST->begin();
	for (; it!=GATE_SERVER_LIST->end(); ++it) {
		MsgServerInfo* si = response.add_servers();
		si->set_name((*it).name());
		si->set_id((*it).id());
		si->set_is_busy((*it).is_busy());
		si->set_is_maintenance((*it).is_maintenance());
	}
	if (!response.SerializeToArray(tmp_, sizeof(tmp_))) {
		LogError("serialize MsgS2C_LoginResponse failed");
		return -1;
	}
	if (agent->sendMsg(MSGID_S2C_LOGIN_RESPONSE, tmp_, response.ByteSize()) < 0) {
		LogError("send MsgS2C_LoginResponse failed");
		return -1;
	}

	LogInfo("account(%s) login", request.account().c_str());
	return info->len;
}

int ClientHandler::processSelectServer(JmyMsgInfo* info)
{
	MsgC2S_SelectServerRequest request;
	if (!request.ParseFromArray(info->data, info->len)) {
		send_error(info, PROTO_ERROR_LOGIN_DATA_INVALID);
		LogError("parse MsgC2LSelectServerRequest failed");
		return -1;
	}

	int gate_id = request.sel_id();
	GateAgent* gate = GATE_SERVER_LIST->getGateAgent(gate_id);
	if (!gate) {
		send_error(info, PROTO_ERROR_LOGIN_GAME_SERVER_MAINTENANCE);
		LogError("cant get gate agent with gate_id(%d)", gate_id);
		return -1;
	}

	std::string account;
	if (!CLIENT_MANAGER->getAccountByConnId(info->conn_id, account)) {
		send_error(info, PROTO_ERROR_LOGIN_DATA_INVALID);
		LogError("cant get account by conn_id(%d)", info->conn_id);
		return -1;
	}

	// send selected messge to gate server
	MsgLS2GT_SelectedServerNotify notify;
	notify.set_account(account);
	if (!notify.SerializeToArray(tmp_, sizeof(tmp_))) {
		LogError("serialize MsgLS2GT_SelectedServerNotify failed");
		return -1;
	}

	if (gate->sendMsg(MSGID_LS2GT_SELECTED_SERVER_NOTIFY, tmp_, notify.ByteSize()) < 0) {
		LogError("send MsgLS2GT_SelectedServerNotify to gate_server(%d) failed", gate_id);
		return -1;
	}

	ClientAgent* client_agent = CLIENT_MANAGER->getAgent(account);
	if (client_agent) {
		client_agent->getData().gate_id = gate_id;
	}
	
	LogInfo("account(%s) conn_id(%d) select server(%d)", account.c_str(), info->conn_id, gate_id);

	return info->len;
}

int ClientHandler::processEcho(JmyMsgInfo* info)
{
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) {
		LogError("get connection failed");
		return -1;
	}

	MsgC2S_EchoRequest request;
	if (!request.ParseFromArray(info->data, info->len)) {
		LogError("parse MsgC2S_EchoRequest failed");
		return -1;
	}

	MsgS2C_EchoResponse response;
	response.set_echo_str(request.echo_str());
	if (!response.SerializeToArray(tmp_, sizeof(tmp_))) {
		LogError("serialize MsgS2C_EchoResponse failed");
		return -1;
	}

#if 0
	if (conn->send(MSGID_S2C_ECHO_RESPONSE, tmp_, response.ByteSize()) < 0) {
		LogError("send MsgS2C_EchoResponse failed");
		return -1;
	}
#endif

	static int count = 0;
	LogInfo("process echo str(%s) count(%d)", response.echo_str().c_str(), ++count);

	return info->len;
}
