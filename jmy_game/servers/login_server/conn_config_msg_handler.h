#pragma once

#include "../libjmy/jmy_const.h"

struct JmyMsgInfo;
class ConnConfigMsgHandler
{
public:
	static int processConnectResponse(JmyMsgInfo*);

private:
	static char tmp_[MAX_SEND_BUFFER_SIZE];
};
