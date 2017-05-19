#pragma once

#include "../libjmy/jmy_const.h"
#include "../common/agent.h"
#include "../common/bi_map.h"
#include "../../proto/src/error.pb.h"
#include <unordered_map>
#include "client_array.h"

enum { RECONN_SESSION_CODE_BUF_LENGTH = 16 };

struct JmyMsgInfo;
struct JmyEventInfo;
class ClientHandler
{
public:
	static int onConnect(JmyEventInfo*);
	static int onDisconnect(JmyEventInfo*);
	static int onTick(JmyEventInfo*);

	static int processGetRoleRequest(JmyMsgInfo*);
	static int processCreateRoleRequest(JmyMsgInfo*);

	static int processEnterGameRequest(JmyMsgInfo*);
	static int processLeaveGameRequest(JmyMsgInfo*);
	static int processReconnectRequest(JmyMsgInfo*);
	static int processDefault(JmyMsgInfo*);

private:
	static char tmp_[JMY_MAX_MSG_SIZE];
	static char session_buf_[RECONN_SESSION_CODE_BUF_LENGTH+1];
};

