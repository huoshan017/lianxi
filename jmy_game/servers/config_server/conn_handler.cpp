#include "conn_handler.h"
#include "../common/util.h"
#include "../../proto/src/server.pb.h"
#include "conf_gate_list.h"

char ConnHandler::tmp_[MAX_SEND_BUFFER_SIZE];
LoginAgentManager ConnHandler::login_mgr_;
GateAgentManager ConnHandler::gate_mgr_;
std::unordered_map<int, int> ConnHandler::conn2agent_map_;
std::set<int> ConnHandler::login_id_set_;
std::set<int> ConnHandler::gate_id_set_;

int ConnHandler::check_conn(int conn_id)
{
	std::unordered_map<int, int>::iterator it = conn2agent_map_.find(conn_id);
	if (it == conn2agent_map_.end()) {
		ServerLogInfo("not found connection %d in set", conn_id);
		return -1;
	}
	if (it->second != 0) {
		ServerLogInfo("connection %d already connected", conn_id);
		return -1;
	}	
	return 0;
}

int ConnHandler::update_conn(int conn_id, int agent_id)
{
	std::unordered_map<int, int>::iterator it = conn2agent_map_.find(conn_id);
	if (it == conn2agent_map_.end())
		return -1;

	it->second = agent_id;
	return 0;
}

int ConnHandler::processGateConnect(JmyMsgInfo* info)
{
	if (check_conn(info->session_id) < 0)
		return -1;

	MsgGT2CS_ConnectRequest request;
	request.ParseFromArray(info->data, info->len);	
	int gate_id = (int)request.gate_id();
	GateAgent* agent = gate_mgr_.getAgent(gate_id);
	if (agent) {
		ServerLogError("already exist gate agent with gate_id(%d) conn_id(%d)", gate_id, info->session_id);
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
	
	update_conn(info->session_id, gate_id);
	gate_id_set_.insert(info->session_id);

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

	ServerLogInfo("gate_server(%d) connected", gate_id);
	return 0;
}

int ConnHandler::processLoginConnect(JmyMsgInfo* info)
{
	if (check_conn(info->session_id) < 0)
		return -1;

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

	update_conn(info->session_id, login_id);
	login_id_set_.insert(info->session_id);	

	MsgCS2LS_ConnectResponse response;
	// get gate server list info from server_list.json
	// and send this list to client for select gate server
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
	ServerLogInfo("login_server(id:%d, ip:%s, port:%d) connected", 
			login_id, login_data.ip.c_str(), login_data.port);
	return 0;
}

int ConnHandler::onConnect(JmyEventInfo* info)
{
	std::unordered_map<int, int>::iterator it = conn2agent_map_.find(info->conn_id);
	if (it != conn2agent_map_.end()) {
		ServerLogError("already has connection %d", info->conn_id);
		return -1;
	}
	conn2agent_map_.insert(std::make_pair(info->conn_id, 0));
	ServerLogInfo("connection %d connected", info->conn_id);
	return 0;
}

int ConnHandler::onDisconnect(JmyEventInfo* info)
{
	std::unordered_map<int, int>::iterator it = conn2agent_map_.find(info->conn_id);
	if (it == conn2agent_map_.end()) {
		ServerLogError("not found connection %d to disconnect", info->conn_id);
		return -1;
	}

	if (login_id_set_.find(info->conn_id) != login_id_set_.end()) {
		LoginAgent* login_agent = login_mgr_.getAgent(it->second);
		if (!login_agent) {
			ServerLogError("not found login agent with login_id(%d) conn_id(%d)", it->second, info->conn_id);
			return -1;
		}
		login_mgr_.deleteAgent(it->second);
		ServerLogInfo("remove login agent with login_id(%d) conn_id(%d)", it->second, info->conn_id);
	} else if (gate_id_set_.find(info->conn_id) != gate_id_set_.end()) {
		GateAgent* gate_agent = gate_mgr_.getAgent(it->second);
		if (!gate_agent) {
			ServerLogError("not found gate agent with gate_id(%d) conn_id(%d)", it->second, info->conn_id);
			return -1;
		}
		gate_mgr_.deleteAgent(it->second);
	} else {
		ServerLogError("connection %d not found agent", info->conn_id);
		return -1;
	}
	conn2agent_map_.erase(it);
	ServerLogInfo("connection %d disconnected", info->conn_id);
	return 0;
}

int ConnHandler::onTick(JmyEventInfo* info)
{
	(void)info;
	return 0;
}

int ConnHandler::onTimer(JmyEventInfo* info)
{
	(void)info;
	return 0;
}
