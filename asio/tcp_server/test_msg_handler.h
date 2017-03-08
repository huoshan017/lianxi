#pragma once

#include <memory>

struct JmyMsgInfo;
class TestMsgHandler
{
public:
	static int process_one(JmyMsgInfo*);

private:
	static uint64_t count_;
};
