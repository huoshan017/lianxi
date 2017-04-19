#pragma once

#include "../libjmy/jmy_const.h"
#include "../common/agent.h"
#include "../common/defines.h"
#include <string>
#include <unordered_map>

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
	
	static bool checkAccountSession(const std::string& account, const std::string& session_code);

private:
	static char tmp_[JMY_MAX_MSG_SIZE];
	static char session_code_buff_[ENTER_GAME_SESSION_CODE_LENGTH+1];
};

#define LOGIN_MGR (ConnLoginHandler::getLoginManager())
