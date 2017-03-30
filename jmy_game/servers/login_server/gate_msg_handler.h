#pragma once

#include "../libjmy/jmy_const.h"
#include "../common/agent.h"
#include <string>

struct GateData {
	int gate_id;
	std::string ip;
	short port;
};
typedef Agent<GateData, int> GateAgent;
typedef AgentManagerPerf<GateData, int, 100> GateAgentManager;

struct JmyMsgInfo;
class GateMsgHandler
{
public:
	static int processConnect(JmyMsgInfo* info);
	static int processSelectedServerResponse(JmyMsgInfo* info);
	static GateAgentManager& getGateManager() { return gate_mgr_; }

private:
	static char tmp_[MAX_SEND_BUFFER_SIZE];
	static GateAgentManager gate_mgr_;
};

#define GATE_MGR (GateMsgHandler::getGateManager())
