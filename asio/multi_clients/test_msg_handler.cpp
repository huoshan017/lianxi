#include "test_msg_handler.h"
#if USE_CONNECTOR_AND_SESSION
#include "../libjmy/jmy_tcp_connector.h"
#else
#include "../libjmy/jmy_tcp_connection.h"
#endif
#include "const_data.h"
#include "util.h"

#if USE_CONNECTOR_AND_SESSION
std::unordered_map<JmyTcpConnectorMgr*, std::unordered_map<int, int> > TestMsgHandler::mgr2id_;
#else
#endif

int TestMsgHandler::process_one(JmyMsgInfo* info)
{
	if (!info) return -1;
	const char* data = info->data;
	unsigned int len = info->len;
	int cid = info->session_id;

#if USE_CONNECTOR_AND_SESSION
	JmyTcpConnectorMgr* mgr = (JmyTcpConnectorMgr*)info->param;
	JmyTcpConnector* conn = mgr->get(cid);
	if (!conn) {
		ClientLogError("get connector(%d) failed", cid);
		return 0;
	}
#else
#endif

	int s = sizeof(s_send_data)/sizeof(s_send_data[0]);
	
#if USE_CONNECTOR_AND_SESSION
	long index = (long)conn->getUnusedData();
	if (std::memcmp(data, s_send_data[index], len) != 0) {
		ClientLogWarn("get data from msg(%s) compared from s_send_data[%d] (%s) is different", data, index, s_send_data[index]);
	} else {
		ClientLogDebug("compare data(%s) is same", data);
	}
	index += 1;
	if (index >= s) {
		index = 0;
	}
	conn->setUnusedData((void*)index);
#else
#endif
	return len;
}
