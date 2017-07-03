#include "conn_config_handler.h"
#include "../common/agent.h"
#include "../common/util.h"
#include "../../proto/src/server.pb.h"
#include "gate_server.h"
#include "config_data.h"
#include "config_loader.h"
#include "global_data.h"

char ConnConfigHandler::tmp_[JMY_MAX_MSG_SIZE];

int ConnConfigHandler::onConnect(JmyEventInfo* info)
{
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) {
		LogError("get connection by info failed");
		return -1;
	}
	MsgGT2CS_ConnectConfigRequest request;
	request.set_gate_id(CONFIG_FILE.id);
	//request.set_gate_ip(CONFIG_FILE.ip.c_str());
	//request.set_gate_port(CONFIG_FILE.port);
	if (!request.SerializeToArray(tmp_, sizeof(tmp_))) {
		LogError("serialize MsgGT2CS_ConnectConfigRequest failed");
		return -1;
	}
	if (conn->send(MSGID_GT2CS_CONNECT_CONFIG_REQUEST, tmp_, request.ByteSize()) < 0) {
		LogError("send MsgGT2CS_ConnectConfigRequest(id:%d, ip:%s, port:%d) failed", CONFIG_FILE.id, CONFIG_FILE.ip.c_str(), CONFIG_FILE.port);
		return -1;
	}
	LogInfo("connection to config_server with conn_id(%d), send gate(id:%d, ip:%s, port:%d)",
			info->conn_id, CONFIG_FILE.id, CONFIG_FILE.ip.c_str(), CONFIG_FILE.port);
	return 0;
}

int ConnConfigHandler::onDisconnect(JmyEventInfo* info)
{
	LogInfo("connection to config_server with conn_id(%d) disconnected", info->conn_id);
	return 0;
}

int ConnConfigHandler::onTick(JmyEventInfo* info)
{
	(void)info;
	return 0;
}

// connect all login server with login list info
int ConnConfigHandler::processConnectConfigResponse(JmyMsgInfo* info)
{
	MsgCS2GT_ConnectConfigResponse response;
	if (!response.ParseFromArray(info->data, info->len)) {
		LogError("parse MsgCS2GT_ConnectConfigResponse failed");
		return -1;
	}
	size_t i = 0;
	size_t s = response.login_list_size();
	for (; i<s; ++i) {
		const MsgLoginInfoData& info = response.login_list(i);
		JmyTcpClient* client = GATE_SERVER->startLoginClient(info.login_ip().c_str(), info.login_port());
		if (!client) {
			LogError("start client to connect login_server(ip:%s, port:%d) failed", info.login_ip().c_str(), info.login_port());
		} else {
			GLOBAL_DATA->addLoginClient(info.login_id(), client);
			LogInfo("start client to connect login_server(ip:%s, port:%d)", info.login_ip().c_str(), info.login_port());
		}
	}
	LogInfo("processed connect config_server response");
	return 0;
}

int ConnConfigHandler::processNewLoginNotify(JmyMsgInfo* info)
{
	MsgCS2GT_NewLoginNotify notify;
	if (!notify.ParseFromArray(info->data, info->len)) {
		LogError("parse MsgCS2GT_NewLoginNotify failed");
		return -1;
	}
	char* login_ip = (char*)notify.login_data().login_ip().c_str();
	unsigned short login_port = notify.login_data().login_port(); 
	JmyTcpClient* client = GATE_SERVER->startLoginClient(login_ip, login_port);
	if (!client) {
		LogError("start client to connect login_server(ip:%s, port:%d) failed", login_ip, login_port);
		return -1;
	}
	GLOBAL_DATA->addLoginClient(notify.login_data().login_id(), client);
	LogInfo("new login_server(ip:%s, port:%d) notified", login_ip, login_port);
	return 0;
}

int ConnConfigHandler::processRemoveLoginNotify(JmyMsgInfo* info)
{
	(void)info;
	return 0;
}
