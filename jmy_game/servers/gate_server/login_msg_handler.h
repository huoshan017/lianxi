#pragma once

#include "../libjmy/jmy_const.h"
#include "../common/agent.h"
#include <string>
#include <unordered_map>

struct LoginAgentData {
};
typedef Agent<LoginAgentData, int> LoginAgent;
typedef AgentManagerPerf<LoginAgentData, int, 100> LoginAgentManager;

enum { ENTER_GAME_SESSION_CODE_LENGTH = 16 };

struct JmyMsgInfo;
class LoginMsgHandler
{
public:
	struct AccountData {
		std::string session_code;
	};
	typedef std::unordered_map<std::string, AccountData*> account2data_type;

	static int processSelectedServerNotify(JmyMsgInfo* info);

	static LoginAgentManager& getLoginManager() { return login_mgr_; }

	static AccountData* getAccountData(const std::string& account);
	static bool removeAccountData(const std::string& account);

private:
	static char tmp_[MAX_SEND_BUFFER_SIZE];
	static LoginAgentManager login_mgr_;
	static account2data_type account2datas_;
	static char session_code_buff_[ENTER_GAME_SESSION_CODE_LENGTH];
};

#define LOGIN_MGR (LoginMsgHandler::getLoginManager())
