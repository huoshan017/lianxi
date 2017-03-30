#include "gate_msg_handler.h"
#include "../common/util.h"
#include "../../proto/src/common.pb.h"
#include "../../proto/src/error.pb.h"
#include "../../proto/src/server.pb.h"
#include "client_msg_handler.h"

char GateMsgHandler::tmp_[MAX_SEND_BUFFER_SIZE];
GateAgentManager GateMsgHandler::gate_mgr_;

int GateMsgHandler::processConnect(JmyMsgInfo* info)
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

	MsgT2LConnectRequest request;
	request.gate_server_id();

	MsgL2TConnectResponse response;
	response.SerializeToArray(tmp_, sizeof(tmp_));
	agent->sendMsg(MSGID_L2T_CONNECT_RESPONSE, tmp_, response.ByteSize());
	return 0;
}

int GateMsgHandler::processSelectedServerResponse(JmyMsgInfo* info)
{
	GateAgent* agent = gate_mgr_.getAgent(info->session_id);
	if (!agent) {
		ServerLogError("cant get gate agent with id(%d)", info->session_id);
		return -1;
	}

	MsgT2LSelectedServerResponse res;
	res.ParseFromArray(info->data, info->len);
	ClientAgent* client_agent = CLIENT_MGR.getAgent(res.account());
	if (!client_agent) {
		ServerLogError("cant get client agent with account(%s)", res.account().c_str());
		return -1;
	}
	
	MsgL2CSelectServerResponse response;
	response.set_account(res.account());
	response.set_session_code(res.session_code());
	response.SerializeToArray(tmp_, sizeof(tmp_));
	client_agent->sendMsg(MSGID_L2C_SELECT_SERVER_RESPONSE, tmp_, response.ByteSize());
	return 0;
}
