#pragma once

#include "../libjmy/jmy_const.h"
#include "../common/agent.h"
#include "../common/defines.h"
#include <string>
#include <unordered_map>

struct LoginAgentData {
};
typedef Agent<LoginAgentData, int> LoginAgent;
typedef AgentManagerPerf<LoginAgentData, int, int, LOGIN_SERVER_MAX_ID, LOGIN_SERVER_MIN_ID> LoginAgentManager;

enum { ENTER_GAME_SESSION_CODE_LENGTH = 16 };

struct JmyMsgInfo;
struct JmyEventInfo;
class ConnLoginHandler
{
public:
	static int processConnectLoginResponse(JmyMsgInfo* info);
	static int processSelectedServerNotify(JmyMsgInfo* info);
	static int onConnect(JmyEventInfo* info);
	static int onDisconnect(JmyEventInfo* info);
	static int onTick(JmyEventInfo* info);
	static int onTimer(JmyEventInfo* info);

	static LoginAgentManager& getLoginManager() { return login_mgr_; }

	static bool checkAccountSession(const std::string& account, const std::string& session_code);

private:
	static char tmp_[MAX_SEND_BUFFER_SIZE];
	static LoginAgentManager login_mgr_;
	static char session_code_buff_[ENTER_GAME_SESSION_CODE_LENGTH];
};

#define LOGIN_MGR (ConnLoginHandler::getLoginManager())
