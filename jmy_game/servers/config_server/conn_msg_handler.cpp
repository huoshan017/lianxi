#include "conn_msg_handler.h"
#include "../common/util.h"
#include "../../proto/src/server.pb.h"
#include "conf_gate_list.h"

char ConnMsgHandler::tmp_[MAX_SEND_BUFFER_SIZE];
LoginAgentManager ConnMsgHandler::login_mgr_;
GateAgentManager ConnMsgHandler::gate_mgr_;
std::set<int> ConnMsgHandler::login_id_set_;
std::set<int> ConnMsgHandler::gate_id_set_;

int ConnMsgHandler::processGateConnect(JmyMsgInfo* info)
{
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) return -1;
	MsgGT2CS_ConnectRequest request;
	request.ParseFromArray(info->data, info->len);
	int gate_id = (int)request.gate_id();
	GateAgent* agent = gate_mgr_.getAgent(gate_id);
	if (agent) {
		ServerLogError("already exist gate agent with id(%d)", gate_id);
		return -1;
	}
	agent = gate_mgr_.newAgent(gate_id, (JmyTcpConnectionMgr*)info->param, info->session_id);
	if (!agent) {
		ServerLogError("create new gate agent with id(%d) session_id(%d) failed", gate_id, info->session_id);
		return -1;
	}

	GateAgentData& gate_data = agent->getData();
	gate_data.id = gate_id;
	gate_data.ip = request.gate_ip();
	gate_data.port = request.gate_port();
	gate_id_set_.insert(gate_id);

	MsgCS2GT_ConnectResponse response;
	// return logins ip and port list to gate server
	std::set<int>::iterator it = login_id_set_.begin();
	for (; it!=login_id_set_.end(); ++it) {
		LoginAgent* login_agent = login_mgr_.getAgent(*it);
		if (!login_agent) continue;
		LoginAgentData& login_data = login_agent->getData();
		MsgLoginInfoData* data = response.add_login_list();
		data->set_login_ip(login_data.ip);
		data->set_login_port(login_data.port);
	}
	if (!response.SerializeToArray(tmp_, sizeof(tmp_))) {
		ServerLogError("serialize connect config_server response message failed");
		return -1;
	}

	if (agent->sendMsg(MSGID_CS2GT_CONNECT_RESPONSE, tmp_, response.ByteSize()) < 0) {
		ServerLogError("send connect config_server response message failed");
		return -1;
	}
	return 0;
}

int ConnMsgHandler::processLoginConnect(JmyMsgInfo* info)
{
	MsgLS2CS_ConnectRequest request;
	if (!request.ParseFromArray(info->data, info->len)) {
		ServerLogError("parse connect config_server request message failed");
		return -1;
	}
	int login_id = (int)request.login_id();
	LoginAgent* agent = login_mgr_.getAgent(login_id);
	if (agent) {
		ServerLogError("already exist login agent with id(%d)", login_id);
		return -1;
	}
	agent = login_mgr_.newAgent(login_id, (JmyTcpConnectionMgr*)info->param, info->session_id);
	if (!agent) {
		ServerLogError("create new login agent with id(%d) session_id(%d) failed", login_id, info->session_id);
		return -1;
	}

	LoginAgentData& login_data = agent->getData();
	login_data.id = login_id;
	login_data.ip = request.login_ip();
	login_data.port = request.login_port();
	login_id_set_.insert(login_id);

	MsgCS2LS_ConnectResponse response;
	// get gate server info from server_list.json
	size_t s = CONF_GATE_LIST->getSize();
	for (size_t i=0; i<s; ++i) {
		ConfGateList::GateData* gate_data = CONF_GATE_LIST->get(i);
		if (!gate_data) continue;
		MsgGateInfoData* data = response.add_gate_list();
		data->set_gate_ip(gate_data->ip);
		data->set_gate_port(gate_data->port);
		data->set_gate_id(gate_data->id);
	}
	if (!response.SerializeToArray(tmp_, sizeof(tmp_))) {
		ServerLogError("serialize connect config_server response message failed");
		return -1;
	}

	if (agent->sendMsg(MSGID_CS2LS_CONNECT_RESPONSE, tmp_, response.ByteSize()) < 0) {
		ServerLogError("send connect config_server response message failed");
		return -1;
	}
	return 0;
}
