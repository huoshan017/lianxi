#pragma once

#include <memory>

class JmyMsgInfo;
class TestMsgHandler
{
public:
	static int process_one(JmyMsgInfo*);
};
