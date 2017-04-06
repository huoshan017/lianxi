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
	static int broadcast_msg_to_gate(int msg_id, char* data, int len);
	static int broadcast_new_login_to_gate(int login_id, const char* ip, unsigned short port);
	static int broadcast_remove_login_to_gate(int login_id);
	//static int broadcast_msg_to_login(int msg_id, char* data, int len);
	//static int broadcast_new_gate_to_login(int gate_id);
	//static int broadcast_remove_gate_to_login(int gate_id);

private:
	static char tmp_[MAX_SEND_BUFFER_SIZE];
	static LoginAgentManager login_mgr_;
	static GateAgentManager gate_mgr_;
	static std::unordered_map<int, int> conn2agent_map_;
	static std::set<int> login_conn_id_set_;
	static std::set<int> gate_conn_id_set_;
};
