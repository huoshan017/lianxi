#pragma once

#include <memory>
#include "jmy_tcp_session.h"

class TestMsgHandler
{
public:
	static int process_one(JmyMsgInfo*);
};
