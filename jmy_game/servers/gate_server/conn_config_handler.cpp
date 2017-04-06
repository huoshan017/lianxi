#include "conn_config_handler.h"
#include "../common/agent.h"
#include "../common/util.h"
#include "../../proto/src/server.pb.h"
#include "gate_server.h"
#include "config_data.h"
#include "config_loader.h"

char ConnConfigHandler::tmp_[MAX_SEND_BUFFER_SIZE];

// connect all login server with login list info
int ConnConfigHandler::processConnectResponse(JmyMsgInfo* info)
{
	MsgCS2GT_ConnectResponse response;
	if (!response.ParseFromArray(info->data, info->len)) {
		ServerLogError("parse MsgCS2GT_ConnectResponse failed");
		return -1;
	}
	size_t i = 0;
	size_t s = response.login_list_size();
	for (; i<s; ++i) {
		const MsgLoginInfoData& info = response.login_list(i);
		s_login_config.conn_ip = (char*)info.login_ip().c_str();
		s_login_config.conn_port = (unsigned short)info.login_port();
		if (!GATE_SERVER->startClient(s_login_config)) {
			ServerLogError("start client to connect login_server(ip:%s, port:%d) failed", info.login_ip().c_str(), info.login_port());
		}
	}
	return 0;
}

int ConnConfigHandler::processNewLoginNotify(JmyMsgInfo* info)
{
	MsgCS2GT_NewLoginNotify notify;
	if (!notify.ParseFromArray(info->data, info->len)) {
		ServerLogError("parse MsgCS2GT_NewLoginNotify failed");
		return -1;
	}
	s_login_config.conn_ip = (char*)notify.login_data().login_ip().c_str();
	s_login_config.conn_port = notify.login_data().login_port();
	if (!GATE_SERVER->startClient(s_login_config)) {
		ServerLogError("start client to connect login_server(ip:%s, port:%d) failed", s_login_config.conn_ip, s_login_config.conn_port);
		return -1;
	}
	ServerLogInfo("new login_server(ip:%s, port:%d) notified", s_login_config.conn_ip, s_login_config.conn_port);
	return 0;
}

int ConnConfigHandler::onConnect(JmyEventInfo* info)
{
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) return -1;
	MsgGT2CS_ConnectRequest request;
	request.set_gate_id(CONFIG_FILE.id);
	request.set_gate_ip(CONFIG_FILE.ip.c_str());
	request.set_gate_port(CONFIG_FILE.port);
	if (!request.SerializeToArray(tmp_, sizeof(tmp_))) {
		ServerLogError("serialize MsgGT2CS_ConnectRequest failed");
		return -1;
	}
	if (conn->send(MSGID_GT2CS_CONNECT_REQUEST, tmp_, request.ByteSize()) < 0) {
		ServerLogError("send MsgGT2CS_ConnectRequest(id:%d, ip:%s, port:%d) failed", CONFIG_FILE.id, CONFIG_FILE.ip.c_str(), CONFIG_FILE.port);
		return -1;
	}
	ServerLogInfo("new connection with conn_id(%d)", info->conn_id);
	return 0;
}

int ConnConfigHandler::onDisconnect(JmyEventInfo* info)
{
	return 0;
}

int ConnConfigHandler::onTick(JmyEventInfo* info)
{
	return 0;
}

int ConnConfigHandler::onTimer(JmyEventInfo* info)
{
	return 0;
}
