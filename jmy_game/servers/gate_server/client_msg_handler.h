#pragma once

#include "../libjmy/jmy_const.h"
#include "../../proto/src/error.pb.h"

struct JmyMsgInfo;
class ClientMsgHandler
{
public:
	static int processEnterGame(JmyMsgInfo*);

private:
	static void send_error(JmyMsgInfo*, ProtoErrorType);
private:
	static char tmp_[MAX_SEND_BUFFER_SIZE];
};
