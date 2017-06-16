#pragma once

#include "../libjmy/jmy_const.h"
#include <string>

struct JmyMsgInfo;
struct JmyEventInfo;
class JmyTcpConnection;
class GameHandler
{
public:
	static int onConnect(JmyEventInfo*);
	static int onDisconnect(JmyEventInfo*);
	static int onTick(JmyEventInfo*);

	static int processGetRole(JmyMsgInfo*);
	static int processCreateRole(JmyMsgInfo*);
	static int processEnterGame(JmyMsgInfo*);
	static int processEnterGameComplete(JmyMsgInfo*);
	static int processReconnect(JmyMsgInfo*);
	static int processDefault(JmyMsgInfo*);
	static int processError(JmyMsgInfo*);

	static void setEnterSession(const std::string& enter_session);

private:
	static int send_enter_game_request(JmyTcpConnection* conn);

private:
	static char tmp_[JMY_MAX_MSG_SIZE];
	//static std::string enter_session_;
	//static std::string reconn_session_;
};
