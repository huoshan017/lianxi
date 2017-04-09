#include "gate_handler.h"
#include "../common/util.h"
#include "../../proto/src/common.pb.h"
#include "../../proto/src/error.pb.h"
#include "../../proto/src/server.pb.h"
#include "client_handler.h"
#include "config_loader.h"

char GateHandler::tmp_[JMY_MAX_MSG_SIZE];
GateAgentManager GateHandler::gate_mgr_;

int GateHandler::init()
{
	gate_mgr_.init();
	return 0;
}

int GateHandler::onConnect(JmyEventInfo* info)
{
	(void)info;
	return 0;
}

int GateHandler::onDisconnect(JmyEventInfo* info)
{
	GateAgent* agent = gate_mgr_.getAgentByConnId(info->conn_id);
	if (!agent) {
		ServerLogError("cant get gate agent with id(%d)", info->conn_id);
		return -1;
	}
	gate_mgr_.deleteAgentByConnId(info->conn_id);
	ServerLogInfo("gate agent (gate_id:%d, conn_id:%d) disconnect", agent->getId(), info->conn_id);
	return 0;
}

int GateHandler::onTick(JmyEventInfo* info)
{
	(void)info;
	return 0;
}

int GateHandler::onTimer(JmyEventInfo* info)
{
	(void)info;
	return 0;
}

const GateData* GateHandler::getGateData(int gate_id)
{
	GateAgent* agent = gate_mgr_.getAgent(gate_id);
	if (!agent)
		return nullptr;
	return &agent->getData();
}

int GateHandler::processConnectLogin(JmyMsgInfo* info)
{
	MsgGT2LS_ConnectLoginRequest request;
	if (!request.ParseFromArray(info->data, info->len)) {
		ServerLogError("parse MsgGT2LS_ConnectLoginRequest failed");
		return -1;
	}

	int gate_id = request.gate_server_id();
	GateAgent* agent = gate_mgr_.newAgent(gate_id, (JmyTcpConnectionMgr*)info->param, info->session_id);
	if (!agent) {
		ServerLogError("cant create new gate agent with id(%d)", info->session_id);
		return -1;
	}

	// new gate agent data
	GateData& data = agent->getData();
	data.id = gate_id;
	data.ip = request.gate_server_ip();
	data.port = request.gate_server_port();

	MsgLS2GT_ConnectLoginResponse response;
	response.set_login_id(SERVER_CONFIG_FILE.id);
	response.SerializeToArray(tmp_, sizeof(tmp_));
	agent->sendMsg(MSGID_LS2GT_CONNECT_LOGIN_RESPONSE, tmp_, response.ByteSize());

	ServerLogInfo("gate_server: %d connected", gate_id);
	return 0;
}

// selected server response from gate server, back message to client
int GateHandler::processSelectedServerResponse(JmyMsgInfo* info)
{
	GateAgent* agent = gate_mgr_.getAgentByConnId(info->session_id);
	if (!agent) {
		ServerLogError("cant get gate agent with id(%d)", info->session_id);
		return -1;
	}

	MsgGT2LS_SelectedServerResponse res;
	res.ParseFromArray(info->data, info->len);
	ClientAgent* client_agent = CLIENT_MGR.getAgent(res.account());
	if (!client_agent) {
		ServerLogError("cant get client agent with account(%s)", res.account().c_str());
		return -1;
	}
	
	MsgS2C_SelectServerResponse response;
	response.set_session_code(res.session_code());
	response.SerializeToArray(tmp_, sizeof(tmp_));
	if (client_agent->sendMsg(MSGID_S2C_SELECT_SERVER_RESPONSE, tmp_, response.ByteSize()) < 0)
		return -1;

	// select server complete, close client
	if (client_agent->close() < 0)
		return -1;

	return 0;
}
