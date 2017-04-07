#pragma once

#include "../libjmy/jmy_const.h"
#include "../../proto/src/common.pb.h"
#include "../../proto/src/error.pb.h"
#include "../common/agent.h"
#include <string>

struct ClientData {
};
typedef Agent<ClientData, int> ClientAgent;
typedef AgentManager<std::string, ClientData, int> ClientAgentManager;

struct JmyMsgInfo;
struct JmyEventInfo;
class JmyTcpConnection;
class ClientHandler
{
public:
	static int processLogin(JmyMsgInfo*);
	static int processSelectServer(JmyMsgInfo*);
	static int onConnect(JmyEventInfo*);
	static int onDisconnect(JmyEventInfo*);
	static int onTick(JmyEventInfo*);
	static int onTimer(JmyEventInfo*);	

	static ClientAgentManager& getClientManager() { return client_mgr_; }
	static ClientAgent* getClientAgent(const std::string& account);

private:
	static void send_error(JmyMsgInfo* info, ProtoErrorType error);
private:
	static char tmp_[JMY_MAX_MSG_SIZE];
	static ClientAgentManager client_mgr_;
};

#define CLIENT_MGR (ClientHandler::getClientManager())
