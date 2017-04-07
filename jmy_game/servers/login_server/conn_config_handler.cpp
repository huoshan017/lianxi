#include "conn_config_handler.h"
#include "../libjmy/jmy_datatype.h"
#include "../libjmy/jmy_tcp_connection.h"
#include "../common/util.h"
#include "config_loader.h"

std::list<MsgGateConfData> ConnConfigHandler::gate_conf_list_;
char ConnConfigHandler::tmp_[MAX_SEND_BUFFER_SIZE];

int ConnConfigHandler::onConnect(JmyEventInfo* info)
{
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) {
		return -1;
	}
	// send msg to config_server
	MsgLS2CS_ConnectConfigRequest request;
	request.set_login_id(SERVER_CONFIG_FILE.id);
	request.set_login_ip(SERVER_CONFIG_FILE.listen_gate_ip);
	request.set_login_port(SERVER_CONFIG_FILE.listen_gate_port);

	if (!request.SerializeToArray(tmp_, sizeof(tmp_))) {
		ServerLogError("serialize message MsgLS2CS_ConnectConfigRequest failed");
		return -1;
	}

	// connect request to get gate_conf_list
	if (conn->send(MSGID_LS2CS_CONNECT_CONFIG_REQUEST, tmp_, request.ByteSize()) < 0) {
		ServerLogError("send MsgLS2CS_ConnectConfigRequest to config_server failed");
		return -1;
	}

	ServerLogInfo("login_server(%d) onconnected config_server", SERVER_CONFIG_FILE.id);
	return 0;
}

int ConnConfigHandler::onDisconnect(JmyEventInfo* info)
{
	(void)info;
	ServerLogInfo("login_server(%d) ondisconnected to config_server", SERVER_CONFIG_FILE.id);
	return 0;
}

int ConnConfigHandler::onTick(JmyEventInfo* info)
{
	(void)info;
	return 0;
}

int ConnConfigHandler::onTimer(JmyEventInfo* info)
{
	(void)info;
	return 0;
}

// get gate_conf_list generated with server_list.json
int ConnConfigHandler::processConnectConfigResponse(JmyMsgInfo* info)
{
	MsgCS2LS_ConnectConfigResponse response;
	if (!response.ParseFromArray(info->data, info->len)) {
		ServerLogError("serilize message MsgCS2LS_ConnectConfigResponse failed");		
		return -1;
	}	

	gate_conf_list_.clear();
	size_t i = 0;
	size_t s = response.gate_list_size();
	for (; i<s; ++i) {
		const MsgGateConfData& gd = response.gate_list(i);
		gate_conf_list_.push_back(gd);
	}

	ServerLogInfo("connect response get gate server list");
	return 0;
}

// notify gate_conf_list
int ConnConfigHandler::processGateConfListNotify(JmyMsgInfo* info)
{
	MsgCS2LS_GateConfListNotify notify;
	if (!notify.ParseFromArray(info->data, info->len)) {
		ServerLogError("serialize MsgCS2LS_GateConfListNotify failed");
		return -1;
	}

	gate_conf_list_.clear();
	size_t i = 0;
	size_t s = notify.gate_list_size();
	for (; i<s; ++i) {
		const MsgGateConfData& gd = notify.gate_list(i);
		gate_conf_list_.push_back(gd);
	}

	ServerLogInfo("config_server notify gate_conf_list");
	return 0;
}
