#pragma once

#include <memory>
#include <unordered_map>

struct JmyMsgInfo;
class JmyTcpConnectorMgr;
class TestMsgHandler
{
public:
	static int process_one(JmyMsgInfo*);

private:
	static std::unordered_map<JmyTcpConnectorMgr*, std::unordered_map<int, int> > mgr2id_;
};
