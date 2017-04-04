#include "gate_handler.h"
#include "../common/util.h"
#include "../../proto/src/common.pb.h"
#include "../../proto/src/error.pb.h"
#include "../../proto/src/server.pb.h"
#include "client_handler.h"

char GateHandler::tmp_[MAX_SEND_BUFFER_SIZE];
GateAgentManager GateHandler::gate_mgr_;

int GateHandler::processConnect(JmyMsgInfo* info)
{
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) {
		return -1;
	}

	GateAgent* agent = gate_mgr_.newAgent(info->session_id, (JmyTcpConnectionMgr*)info->param);
	if (!agent) {
		ServerLogError("cant create new gate agent with id(%d)", info->session_id);
		return -1;
	}

	MsgGT2LS_ConnectRequest request;
	request.gate_server_id();

	MsgLS2GT_ConnectResponse response;
	response.SerializeToArray(tmp_, sizeof(tmp_));
	agent->sendMsg(MSGID_LS2GT_CONNECT_RESPONSE, tmp_, response.ByteSize());
	return 0;
}

int GateHandler::processSelectedServerResponse(JmyMsgInfo* info)
{
	GateAgent* agent = gate_mgr_.getAgent(info->session_id);
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
	
	MsgLS2CL_SelectServerResponse response;
	response.set_session_code(res.session_code());
	response.SerializeToArray(tmp_, sizeof(tmp_));
	if (client_agent->sendMsg(MSGID_LS2CL_SELECT_SERVER_RESPONSE, tmp_, response.ByteSize()) < 0)
		return -1;

	// select server complete, close client
	if (client_agent->close() < 0)
		return -1;

	return 0;
}

int GateHandler::onConnect(JmyEventInfo* info)
{
	return 0;
}

int GateHandler::onDisconnect(JmyEventInfo* info)
{
	return 0;
}

int GateHandler::onTick(JmyEventInfo* info)
{
	return 0;
}

int GateHandler::onTimer(JmyEventInfo* info)
{
	return 0;
}
