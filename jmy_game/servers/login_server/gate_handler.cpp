#include "gate_handler.h"
#include "../common/util.h"
#include "../../proto/src/common.pb.h"
#include "../../proto/src/error.pb.h"
#include "../../proto/src/server.pb.h"
#include "client_handler.h"
#include "client_manager.h"
#include "config_loader.h"
#include "gate_server_list.h"

char GateHandler::tmp_[JMY_MAX_MSG_SIZE];

int GateHandler::onConnect(JmyEventInfo* info)
{
	LogInfo("new gate server(conn_id:%d) onConnect", info->conn_id);
	return 0;
}

int GateHandler::onDisconnect(JmyEventInfo* info)
{
	if (!GATE_SERVER_LIST->deleteGateAgentByConnId(info->conn_id)) {
		LogError("detele gate agent(conn_id:%d) failed", info->conn_id);
		return -1;
	}
	LogInfo("gate agent (conn_id:%d) onDisconnect", info->conn_id);
	return 0;
}

int GateHandler::onTick(JmyEventInfo* info)
{
	(void)info;
	return 0;
}

int GateHandler::processConnectLogin(JmyMsgInfo* info)
{
	MsgGT2LS_ConnectLoginRequest request;
	if (!request.ParseFromArray(info->data, info->len)) {
		LogError("parse MsgGT2LS_ConnectLoginRequest failed");
		return -1;
	}

	int gate_id = request.gate_server_id();
	GateAgent* gate_agent = GATE_SERVER_LIST->getGateAgent(gate_id);
	if (gate_agent) {
		LogError("gate agent %d already exists", gate_id);
		return -1;
	}

	gate_agent = GATE_SERVER_LIST->newGateAgent(gate_id, info);
	if (!gate_agent) {
		LogError("create new gate agent with gate_id(%d) failed", gate_id);
		return -1;
	}

	// new gate agent data
	GateData& data = gate_agent->getData();
	data.id = gate_id;
	data.ip = request.gate_server_ip();
	data.port = request.gate_server_port();

	MsgLS2GT_ConnectLoginResponse response;
	response.set_login_id(SERVER_CONFIG.id);
	if (!response.SerializeToArray(tmp_, sizeof(tmp_))) {
		LogError("serialize MsgLS2GT_ConnectLoginResponse failed");
		return -1;
	}
	if (gate_agent->sendMsg(MSGID_LS2GT_CONNECT_LOGIN_RESPONSE, tmp_, response.ByteSize()) < 0) {
		LogError("send MsgLS2GT_ConnectLoginResponse failed");
		return -1;
	}

	LogInfo("gate_server: %d connected", gate_id);
	return info->len;
}

// selected server response from gate server, back message to client
int GateHandler::processSelectedServerResponse(JmyMsgInfo* info)
{
	MsgGT2LS_SelectedServerResponse res;
	if (!res.ParseFromArray(info->data, info->len)) {
		LogError("parse MsgGT2LS_SelectedServerResponse failed");
		return -1;
	}

	ClientAgent* client_agent = CLIENT_MANAGER->getAgent(res.account());
	if (!client_agent) {
		LogError("cant get client agent with account(%s)", res.account().c_str());
		return -1;
	}
	
	int gate_id = client_agent->getData().gate_id;
	GateAgent* gate_agent = GATE_SERVER_LIST->getGateAgent(gate_id);
	if (!gate_agent) {
		LogError("cant get gate agent with gate_id(%d)", gate_id);
		return -1;
	}
	MsgS2C_SelectServerResponse response;
	response.set_session_code(res.session_code());
	response.set_server_ip(gate_agent->getData().ip);
	response.set_port(gate_agent->getData().port);
	if (!response.SerializeToArray(tmp_, sizeof(tmp_))) {
		LogError("serialize MsgS2C_SelectServerResponse failed");
		return -1;
	}

	if (client_agent->sendMsg(MSGID_S2C_SELECT_SERVER_RESPONSE, tmp_, response.ByteSize()) < 0) {
		LogError("send MsgS2C_SelectServerResponse failed");
		return -1;
	}

#if 0
	// select server complete, close client
	if (client_agent->close() < 0) {
		LogError("client agent(account:%s) close failed", res.account().c_str());
		return -1;
	}
#endif

	static int sel_count = 0;
	LogInfo("account(%s) selected server response with enter_session(%s) to back client, sel_count(%d)", res.account().c_str(), res.session_code().c_str(), ++sel_count);

	return info->len;
}
