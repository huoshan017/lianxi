#pragma once

#include <memory>

struct JmyMsgInfo;
class TestMsgHandler
{
public:
	static int process_one(JmyMsgInfo*);
	static int inc_count() { return count_ += 1; }
	static int get_count() { return count_; }

private:
	static int count_;
};
