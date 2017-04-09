#pragma once

#include "../libjmy/jmy_const.h"
#include "../common/agent.h"
#include "../common/defines.h"
#include <string>
#include <unordered_map>

struct GateData {
	int id;
	std::string ip;
	short port;
};
typedef Agent<GateData, int> GateAgent;
typedef AgentManagerPerf<GateData, int, int, GATE_SERVER_MAX_ID, GATE_SERVER_MIN_ID> GateAgentManager;

struct JmyMsgInfo;
struct JmyEventInfo;
class GateHandler
{
public:
	static int init();
	static int processConnectLogin(JmyMsgInfo* info);
	static int processSelectedServerResponse(JmyMsgInfo* info);
	static int onConnect(JmyEventInfo*);
	static int onDisconnect(JmyEventInfo*);
	static int onTick(JmyEventInfo*);
	static int onTimer(JmyEventInfo*);	

	static GateAgentManager& getGateManager() { return gate_mgr_; }
	static const GateData* getGateData(int gate_id);

private:
	static char tmp_[JMY_MAX_MSG_SIZE];
	static GateAgentManager gate_mgr_;
};

#define GATE_MGR (GateHandler::getGateManager())
