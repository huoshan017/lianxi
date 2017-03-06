#pragma once

#include <memory>
#include "../libjmy/jmy_tcp_session.h"

struct JmyMsgInfo;
class TestMsgHandler
{
public:
	static int process_one(JmyMsgInfo*);

private:
	static uint64_t count_;
};
