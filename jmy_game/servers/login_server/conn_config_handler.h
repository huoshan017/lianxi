#pragma once

#include "../libjmy/jmy_const.h"
#include "../common/agent.h"
#include "../common/defines.h"
#include "../../proto/src/server.pb.h"
#include <list>

struct JmyMsgInfo;
struct JmyEventInfo;
class ConnConfigHandler
{
public:
	static int processConnectConfigResponse(JmyMsgInfo*);
	static int processGateConfListNotify(JmyMsgInfo*);
	static int onConnect(JmyEventInfo*);
	static int onDisconnect(JmyEventInfo*);
	static int onTick(JmyEventInfo*);
	static int onTimer(JmyEventInfo*);

	static const std::list<MsgGateConfData>& getGateConfList() { return gate_conf_list_; }

private:
	static std::list<MsgGateConfData> gate_conf_list_; 
	static char tmp_[JMY_MAX_MSG_SIZE];
};

#define GATE_CONF_LIST (ConnConfigHandler::getGateConfList())
