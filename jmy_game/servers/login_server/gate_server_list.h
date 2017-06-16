#pragma once

#include <string>
#include <list>
#include "../libjmy/jmy_singleton.hpp"
#include "../common/agent.h"
#include "../../proto/src/server.pb.h"

struct GateData {
	int id;
	std::string ip;
	short port;
};
typedef Agent<GateData, int> GateAgent;
typedef AgentManagerPerf<GateData, int, int, GATE_SERVER_MAX_ID, GATE_SERVER_MIN_ID> GateAgentManager;

class GateServerList : public JmySingleton<GateServerList>
{
public:
	GateServerList() {}
	~GateServerList() { clear(); }

	void init();

	void clear() {
		gate_conf_list_.clear();
	}

	void push(MsgGateConfData& data) {
		gate_conf_list_.push_back(std::move(data));
	}
	typedef std::list<MsgGateConfData>::iterator Iterator;
	Iterator begin() {
		return gate_conf_list_.begin();
	}
	Iterator end() {
		return gate_conf_list_.end();
	}

	GateAgent* newGateAgent(int gate_id, JmyMsgInfo* info);
	GateAgent* getGateAgent(int gate_id);
	GateAgent* getGateAgentByConnId(int conn_id);
	bool deleteGateAgent(int gate_id);
	bool deleteGateAgentByConnId(int conn_id);

private:
	std::list<MsgGateConfData> gate_conf_list_;
	GateAgentManager gate_mgr_;
};

#define GATE_SERVER_LIST (GateServerList::getInstance())
