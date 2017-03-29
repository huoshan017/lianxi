#pragma once

#include "../libjmy/jmy_const.h"
#include "../../proto/src/common.pb.h"
#include "../../proto/src/error.pb.h"

struct JmyMsgInfo;
class GateMsgHandler
{
public:
	static int processConnect(JmyMsgInfo* info);

private:
	static char tmp_[MAX_SEND_BUFFER_SIZE];
};
