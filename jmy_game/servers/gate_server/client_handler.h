#pragma once

#include "../libjmy/jmy_const.h"
#include "../common/agent.h"
#include "../../proto/src/error.pb.h"

struct ClientData {
	std::string account;
};
typedef Agent<ClientData, int> ClientAgent;
typedef AgentManager<std::string, ClientData, int> ClientAgentManager;

struct JmyMsgInfo;
class ClientHandler
{
public:
	static int processEnterGame(JmyMsgInfo*);
	static ClientAgentManager& getClientManager() { return client_mgr_; }

private:
	static void send_error(JmyMsgInfo*, ProtoErrorType);
private:
	static char tmp_[MAX_SEND_BUFFER_SIZE];
	static ClientAgentManager client_mgr_;
};

#define CLIENT_MGR (ClientHandler::getClientManager())
