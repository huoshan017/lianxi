#include "gate_server_list.h"
#include "../common/util.h"

void GateServerList::init()
{
	gate_mgr_.init();
}

GateAgent* GateServerList::newGateAgent(int gate_id, JmyMsgInfo* info)
{
	GateAgent* agent = gate_mgr_.newAgent(gate_id, (JmyTcpConnectionMgr*)info->param, info->conn_id);
	if (!agent) {
		LogError("cant create new gate agent with id(%d)", info->conn_id);
		return nullptr;
	}
	return agent;
}

GateAgent* GateServerList::getGateAgent(int gate_id)
{
	return gate_mgr_.getAgent(gate_id);
}

GateAgent* GateServerList::getGateAgentByConnId(int conn_id)
{
	return gate_mgr_.getAgentByConnId(conn_id);
}

bool GateServerList::deleteGateAgent(int gate_id)
{
	return gate_mgr_.deleteAgent(gate_id);
}

bool GateServerList::deleteGateAgentByConnId(int conn_id)
{
	return gate_mgr_.deleteAgentByConnId(conn_id);
}
