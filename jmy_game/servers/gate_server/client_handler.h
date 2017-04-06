#pragma once

#include "../libjmy/jmy_const.h"
#include "../common/agent.h"
#include "../../proto/src/error.pb.h"
#include <unordered_map>

struct ClientData {
	std::string enter_session;
	std::string reconn_session;
};
typedef Agent<ClientData, int> ClientAgent;
typedef AgentManager<std::string, ClientData, int> ClientAgentManager;

struct JmyMsgInfo;
struct JmyEventInfo;
class ClientHandler
{
public:
	static int processEnterGame(JmyMsgInfo*);
	static int processReconnect(JmyMsgInfo*);
	static int onConnect(JmyEventInfo*);
	static int onDisconnect(JmyEventInfo*);
	static int onTick(JmyEventInfo*);
	static int onTimer(JmyEventInfo*);

	static ClientAgentManager& getClientManager() { return client_mgr_; }
	static bool newClientSession(const std::string& account, const std::string& session_code);

private:
	static void send_error(JmyMsgInfo*, ProtoErrorType);
private:
	static char tmp_[MAX_SEND_BUFFER_SIZE];
	static ClientAgentManager client_mgr_;
	static std::unordered_map<std::string, std::string> account2session_map_;
};

#define CLIENT_MGR (ClientHandler::getClientManager())
