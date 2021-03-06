#include "conn_handler.h"
#include "../common/util.h"
#include "../../proto/src/server.pb.h"
#include "conf_gate_list.h"

char ConnHandler::tmp_[JMY_MAX_MSG_SIZE];
LoginAgentManager ConnHandler::login_mgr_;
GateAgentManager ConnHandler::gate_mgr_;
std::unordered_map<int, int> ConnHandler::conn2agent_map_;
std::set<int> ConnHandler::login_conn_id_set_;
std::set<int> ConnHandler::gate_conn_id_set_;

int ConnHandler::onConnect(JmyEventInfo* info)
{
	std::unordered_map<int, int>::iterator it = conn2agent_map_.find(info->conn_id);
	if (it != conn2agent_map_.end()) {
		LogError("already has connection %d", info->conn_id);
		return -1;
	}
	conn2agent_map_.insert(std::make_pair(info->conn_id, 0));
	LogInfo("connection %d connected", info->conn_id);
	return 0;
}

int ConnHandler::onDisconnect(JmyEventInfo* info)
{
	std::unordered_map<int, int>::iterator it = conn2agent_map_.find(info->conn_id);
	if (it == conn2agent_map_.end()) {
		LogError("not found connection %d to disconnect", info->conn_id);
		return -1;
	}

	conn2agent_map_.erase(it);
	if (login_conn_id_set_.find(info->conn_id) != login_conn_id_set_.end()) {
		LoginAgent* login_agent = login_mgr_.getAgent(it->second);
		if (!login_agent) {
			LogError("not found login agent with login_id(%d) conn_id(%d)", it->second, info->conn_id);
			return -1;
		}
		login_mgr_.deleteAgent(it->second);
		login_conn_id_set_.erase(info->conn_id);
		LogInfo("remove login agent with login_id(%d) conn_id(%d)", it->second, info->conn_id);
	} else if (gate_conn_id_set_.find(info->conn_id) != gate_conn_id_set_.end()) {
		GateAgent* gate_agent = gate_mgr_.getAgent(it->second);
		if (!gate_agent) {
			LogError("not found gate agent with gate_id(%d) conn_id(%d)", it->second, info->conn_id);
			return -1;
		}
		gate_mgr_.deleteAgent(it->second);
		gate_conn_id_set_.erase(info->conn_id);
		LogInfo("remove gate agent with gate_id(%d) conn_id(%d)", it->second, info->conn_id);
	} else {
		LogInfo("connection %d not found agent", info->conn_id);
		return 0;
	}

	LogInfo("connection conn_id(%d) disconnected", info->conn_id);
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

int ConnHandler::check_conn(int conn_id)
{
	std::unordered_map<int, int>::iterator it = conn2agent_map_.find(conn_id);
	if (it == conn2agent_map_.end()) {
		LogInfo("not found connection %d in set", conn_id);
		return -1;
	}
	if (it->second != 0) {
		LogInfo("connection %d already connected", conn_id);
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

int ConnHandler::broadcast_msg_to_gate(int msg_id, char* data, int len)
{
	std::unordered_map<int, int>::iterator c2a_it;
	std::set<int>::iterator it = gate_conn_id_set_.begin();
	for (; it!=gate_conn_id_set_.end(); ++it) {
		c2a_it = conn2agent_map_.find(*it);
		if (c2a_it == conn2agent_map_.end()) {
			LogWarn("not found gate_id with conn_id(%d)", *it);
			continue;
		}
		GateAgent* agent = gate_mgr_.getAgent(c2a_it->second);
		if (!agent) {
			LogWarn("not found gate agent with gate_id(%d)", c2a_it->second);
			return -1;
		}
		if (agent->sendMsg(msg_id, data, len) < 0)
			return -1;
		LogInfo("broadcast message(%d) to login_server(%d)", msg_id, c2a_it->second);
	}
	return 0;
}

int ConnHandler::broadcast_new_login_to_gate(int login_id, const char* login_ip, unsigned short login_port)
{
	MsgCS2GT_NewLoginNotify notify;
	notify.mutable_login_data()->set_login_id(login_id);
	notify.mutable_login_data()->set_login_ip(login_ip);
	notify.mutable_login_data()->set_login_port(login_port);
	if (!notify.SerializeToArray(tmp_, sizeof(tmp_)))
		return -1;
	
	return broadcast_msg_to_gate(MSGID_CS2GT_NEW_LOGIN_NOTIFY, tmp_, notify.ByteSize());
}

int ConnHandler::broadcast_remove_login_to_gate(int login_id)
{
	MsgCS2GT_RemoveLoginNotify notify;
	notify.set_login_id(login_id);
	if (!notify.SerializeToArray(tmp_, sizeof(tmp_)))
		return -1;

	return broadcast_msg_to_gate(MSGID_CS2GT_REMOVE_LOGIN_NOTIFY, tmp_, notify.ByteSize());
}

#if 0
int ConnHandler::broadcast_msg_to_login(int msg_id, char* data, int len)
{
	std::unordered_map<int, int>::iterator c2a_it;
	std::set<int>::iterator it = login_conn_id_set_.begin();
	for (; it!=login_conn_id_set_.end(); ++it) {
		c2a_it = conn2agent_map_.find(*it);
		if (c2a_it == conn2agent_map_.end()) {
			ServerLogWarn("not found login_id with conn_id(%d)", *it);
			continue;
		}
		LoginAgent* agent = login_mgr_.getAgent(c2a_it->second);
		if (!agent) {
			ServerLogWarn("not found login agent with login_id(%d)", c2a_it->second);
			return -1;
		}
		if (agent->sendMsg(msg_id, data, len) < 0)
			return -1;
	}
	return 0;
}
#endif

int ConnHandler::processGateConnect(JmyMsgInfo* info)
{
	if (check_conn(info->conn_id) < 0)
		return -1;

	MsgGT2CS_ConnectConfigRequest request;
	request.ParseFromArray(info->data, info->len);	
	int gate_id = (int)request.gate_id();
	GateAgent* agent = gate_mgr_.getAgent(gate_id);
	if (agent) {
		LogError("already exist gate agent with gate_id(%d) conn_id(%d)", gate_id, info->conn_id);
		return -1;
	}
	agent = gate_mgr_.newAgent(gate_id, (JmyTcpConnectionMgr*)info->param, info->conn_id);
	if (!agent) {
		LogError("create new gate agent with id(%d) session_id(%d) failed", gate_id, info->conn_id);
		return -1;
	}
		
	MsgGateConfData* d = CONF_GATE_LIST->getById(gate_id);
	if (!d) {
		LogError("not found gate server id: %d", gate_id);
		return -1;
	}

	GateAgentData& gate_data = agent->getData();
	gate_data.id = gate_id;
	gate_data.ip =  d->ip();//request.gate_ip();
	gate_data.port = d->port(); //request.gate_port();
	
	update_conn(info->conn_id, gate_id);
	gate_conn_id_set_.insert(info->conn_id);

	MsgCS2GT_ConnectConfigResponse response;
	// return logins (ip, port, id) list to gate server
	std::set<int>::iterator it = login_conn_id_set_.begin();
	for (; it!=login_conn_id_set_.end(); ++it) {
		LoginAgent* login_agent = login_mgr_.getAgentByConnId(*it);
		if (!login_agent) continue;
		LoginAgentData& login_data = login_agent->getData();
		MsgLoginInfoData* data = response.add_login_list();
		data->set_login_ip(login_data.ip);
		data->set_login_port(login_data.port);
		data->set_login_id(login_data.id);
	}

	if (!response.SerializeToArray(tmp_, sizeof(tmp_))) {
		LogError("serialize connect config_server response message failed");
		return -1;
	}
	if (agent->sendMsg(MSGID_CS2GT_CONNECT_CONFIG_RESPONSE, tmp_, response.ByteSize()) < 0) {
		LogError("send connect config_server response message failed");
		return -1;
	}

	LogInfo("gate_server(%d) connected", gate_id);
	return 0;
}

int ConnHandler::processLoginConnect(JmyMsgInfo* info)
{
	if (check_conn(info->conn_id) < 0)
		return -1;

	MsgLS2CS_ConnectConfigRequest request;
	if (!request.ParseFromArray(info->data, info->len)) {
		LogError("parse connect config_server request message failed");
		return -1;
	}
	int login_id = (int)request.login_id();
	LoginAgent* agent = login_mgr_.getAgent(login_id);
	if (agent) {
		LogError("already exist login agent with id(%d)", login_id);
		return -1;
	}
	agent = login_mgr_.newAgent(login_id, (JmyTcpConnectionMgr*)info->param, info->conn_id);
	if (!agent) {
		LogError("create new login agent with id(%d) session_id(%d) failed", login_id, info->conn_id);
		return -1;
	}

	LoginAgentData& login_data = agent->getData();
	login_data.id = login_id;
	login_data.ip = request.login_ip();
	login_data.port = request.login_port();

	update_conn(info->conn_id, login_id);
	login_conn_id_set_.insert(info->conn_id);	

	MsgCS2LS_ConnectConfigResponse response;
	// get gate server list info from server_list.json
	// and send this list to client for select gate server
	size_t s = CONF_GATE_LIST->getSize();
	for (size_t i=0; i<s; ++i) {
		MsgGateConfData* gate_data = CONF_GATE_LIST->get(i);
		if (!gate_data) continue;
		MsgGateConfData* data = response.add_gate_list();
		*data = *gate_data;
	}
	if (!response.SerializeToArray(tmp_, sizeof(tmp_))) {
		LogError("serialize connect config_server response message failed");
		return -1;
	}

	if (agent->sendMsg(MSGID_CS2LS_CONNECT_CONFIG_RESPONSE, tmp_, response.ByteSize()) < 0) {
		LogError("send connect config_server response message failed");
		return -1;
	}

	// notify gate new login
	if (broadcast_new_login_to_gate(login_id, login_data.ip.c_str(), login_data.port) < 0)
		return -1;

	LogInfo("login_server(id:%d, ip:%s, port:%d) connected", login_id, login_data.ip.c_str(), login_data.port);
	return 0;
}
