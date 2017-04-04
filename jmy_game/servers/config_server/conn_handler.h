#pragma once

#include "../libjmy/jmy_const.h"
#include "../common/agent.h"
#include <string>
#include <unordered_map>
#include <set>

struct LoginAgentData {
	int id;
	std::string ip;
	short port;
};
typedef Agent<LoginAgentData, int> LoginAgent;
typedef AgentManager<int, LoginAgentData, int> LoginAgentManager;

struct GateAgentData {
	int id;
	std::string ip;
	short port;
};
typedef Agent<GateAgentData, int> GateAgent;
typedef AgentManager<int, GateAgentData, int> GateAgentManager;

struct JmyMsgInfo;
struct JmyEventInfo;
class ConnHandler
{
public:
	static int processLoginConnect(JmyMsgInfo*);
	static int processGateConnect(JmyMsgInfo*);
	static int onConnect(JmyEventInfo*);
	static int onDisconnect(JmyEventInfo*);
	static int onTick(JmyEventInfo*);
	static int onTimer(JmyEventInfo*);

private:
	static int check_conn(int conn_id);
	static int update_conn(int conn_id, int agent_id);
	static int genMsgGateList();

private:
	static char tmp_[MAX_SEND_BUFFER_SIZE];
	static LoginAgentManager login_mgr_;
	static GateAgentManager gate_mgr_;
	static std::unordered_map<int, int> conn2agent_map_;
	static std::set<int> login_id_set_;
	static std::set<int> gate_id_set_;
};
