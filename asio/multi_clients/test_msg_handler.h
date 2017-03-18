#pragma once

#include <memory>
#include <unordered_map>

struct JmyMsgInfo;
#if USE_CONNECTOR_AND_SESSION
class JmyTcpConnectorMgr;
#else
#endif
class TestMsgHandler
{
public:
	static int process_one(JmyMsgInfo*);

private:
#if USE_CONNECTOR_AND_SESSION
	static std::unordered_map<JmyTcpConnectorMgr*, std::unordered_map<int, int> > mgr2id_;
#endif
};
