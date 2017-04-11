#include "conn_gate_handler.h"
#include "../libjmy/jmy_datatype.h"
#include "../libjmy/jmy_tcp_connection.h"
#include "../common/util.h"
#include "../../proto/src/server.pb.h"
#include "config_loader.h"

char ConnGateHandler::tmp_[JMY_MAX_MSG_SIZE];

int ConnGateHandler::onConnect(JmyEventInfo* info)
{
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) return -1;
	MsgGS2GT_ConnectGateRequest request;
	request.set_game_id(SERVER_CONFIG.id);
	if (!request.SerializeToArray(tmp_, sizeof(tmp_))) {
		ServerLogError("serialize MsgGS2GT_ConnectGateRequest failed");
		return -1;
	}
	if (conn->send(MSGID_GS2GT_CONNECT_GATE_REQUEST, tmp_, request.ByteSize()) < 0) {
		ServerLogError("send message MsgGS2GT_ConnectGateRequest failed");
		return -1;
	}
	ServerLogInfo("send message MsgGS2GT_ConnectGateRequest to gate server");
	return 0;
}

int ConnGateHandler::onDisconnect(JmyEventInfo* info)
{
	ServerLogInfo("ondisconnect");
	return 0;
}

int ConnGateHandler::onTick(JmyEventInfo* info)
{
	return 0;
}

int ConnGateHandler::onTimer(JmyEventInfo* info)
{
	return 0;
}

int ConnGateHandler::processEnterGame(JmyMsgInfo* info)
{
	return info->len;
}

int ConnGateHandler::processLeaveGame(JmyMsgInfo* info)
{
	return info->len;
}

int ConnGateHandler::processDefault(JmyMsgInfo* info)
{
	switch (info->msg_id) {
	}
	return info->len;
}
