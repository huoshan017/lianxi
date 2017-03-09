#include "test_msg_handler.h"
#include "../libjmy/jmy_tcp_connector.h"
#include "const_data.h"
#include "util.h"

std::unordered_map<JmyTcpConnectorMgr*, std::unordered_map<int, int> > TestMsgHandler::mgr2id_;

int TestMsgHandler::process_one(JmyMsgInfo* info)
{
	if (!info) return -1;
	const char* data = info->data;
	unsigned int len = info->len;
	int cid = info->session_id;
	JmyTcpConnectorMgr* mgr = (JmyTcpConnectorMgr*)info->param;
	JmyTcpConnector* conn = mgr->get(cid);
	if (!conn) {
		ClientLogError("get connector(%d) failed", cid);
		return 0;
	}

	int s = sizeof(s_send_data)/sizeof(s_send_data[0]);
#if 0
	std::unordered_map<JmyTcpConnectorMgr*, std::unordered_map<int, int> >::iterator it = mgr2id_.find(mgr);
	if (it == mgr2id_.end()) {
		std::unordered_map<int, int> id2idx;
		mgr2id_.insert(std::make_pair(mgr, id2idx));
		it = mgr2id_.find(mgr);
	}
	std::unordered_map<int, int>::iterator iit = it->second.find(cid);
	if (iit == it->second.end()) {
		it->second.insert(std::make_pair(cid, 0));
		iit = it->second.find(cid);
	}
	
	int index = (iit->second);
	iit->second += 1;
	if (iit->second >= s) {
		iit->second = 0;
	}
#endif
	long index = (long)conn->getUnusedData();
	if (std::memcmp(data, s_send_data[index], len) != 0) {
		ClientLogError("get data from msg(%s) compared from s_send_data[%d] (%s) is different", data, index, s_send_data[index]);
	} else {
		ClientLogDebug("compare data(%s) is same", data);
	}
	index += 1;
	if (index >= s) {
		index = 0;
	}
	conn->setUnusedData((void*)index);
	return len;
}
