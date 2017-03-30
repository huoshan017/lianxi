#pragma once

#include "../libjmy/jmy_const.h"
#include "../../proto/src/common.pb.h"
#include "../../proto/src/error.pb.h"
#include "../common/agent.h"
#include <string>

struct ClientData {
	std::string account;
};
typedef Agent<ClientData, int> ClientAgent;
typedef AgentManager<std::string, ClientData, int> ClientAgentManager;

struct JmyMsgInfo;
class JmyTcpConnection;
class ClientMsgHandler
{
public:
	static int processLogin(JmyMsgInfo*);
	static int processSelectServer(JmyMsgInfo*);
	static ClientAgentManager& getClientManager() { return client_mgr_; }
	static ClientAgent* getClientAgent(const std::string& account);

private:
	static void send_error(JmyMsgInfo* info, ProtoErrorType error);
private:
	static char tmp_[MAX_SEND_BUFFER_SIZE];
	static ClientAgentManager client_mgr_;
};

#define CLIENT_MGR (ClientMsgHandler::getClientManager())
