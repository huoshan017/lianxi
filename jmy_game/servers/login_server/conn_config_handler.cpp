#include "conn_config_handler.h"
#include "../libjmy/jmy_datatype.h"
#include "../libjmy/jmy_tcp_connection.h"
#include "../common/util.h"
#include "../../proto/src/server.pb.h"
#include "config_loader.h"

char ConnConfigHandler::tmp_[MAX_SEND_BUFFER_SIZE];

int ConnConfigHandler::processConnectResponse(JmyMsgInfo* info)
{
	MsgCS2LS_ConnectResponse response;
	if (!response.SerializeToArray(info->data, info->len)) {
		ServerLogError("serilize config_server to login_server connect response message failed");		
		return -1;
	}	
	return 0;
}

int ConnConfigHandler::onConnect(JmyEventInfo* info)
{
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) {
		return -1;
	}
	// send msg to config server
	MsgLS2CS_ConnectRequest request;
	request.set_login_id(SERVER_CONFIG_FILE.id);
	request.set_login_ip(SERVER_CONFIG_FILE.listen_gate_ip);
	request.set_login_port(SERVER_CONFIG_FILE.listen_gate_port);

	if (!request.SerializeToArray(tmp_, sizeof(tmp_))) {
		ServerLogError("serialize message failed");
		return -1;
	}

	if (conn->send(MSGID_LS2CS_CONNECT_REQUEST, tmp_, request.ByteSize()) < 0) {
		ServerLogError("send connect response to config_server failed");
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
