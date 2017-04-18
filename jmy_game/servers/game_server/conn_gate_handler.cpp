#include "conn_gate_handler.h"
#include "../libjmy/jmy_datatype.h"
#include "../libjmy/jmy_tcp_connection.h"
#include "../common/util.h"
#include "../../proto/src/server.pb.h"
#include "config_loader.h"
#include "player.h"

char ConnGateHandler::tmp_[JMY_MAX_MSG_SIZE];
JmyTcpConnection* ConnGateHandler::gate_conn_ = nullptr;
std::unordered_map<std::string, int> ConnGateHandler::account2id_map_;

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
	gate_conn_ = conn;
	ServerLogInfo("send message MsgGS2GT_ConnectGateRequest to gate server");
	return 0;
}

int ConnGateHandler::onDisconnect(JmyEventInfo* info)
{
	(void)info;
	gate_conn_ = nullptr;
	ServerLogInfo("ondisconnect");
	return 0;
}

int ConnGateHandler::onTick(JmyEventInfo* info)
{
	(void)info;
	return 0;
}

int ConnGateHandler::onTimer(JmyEventInfo* info)
{
	(void)info;
	return 0;
}

int ConnGateHandler::processConnectGateResponse(JmyMsgInfo* info)
{
	return info->len;
}

int ConnGateHandler::processEnterGame(JmyMsgInfo* info)
{
	MsgGT2GS_EnterGameRequest request;
	if (!request.ParseFromArray(info->data, info->len)) {
		ServerLogError("parse MsgGT2GS_EnterGameRequest failed");
		return -1;
	}
	int user_id = info->user_id;
	Player* p = PLAYER_MGR->get(user_id);
	if (!p) {
		account2id_map_.insert(std::make_pair(request.account(), user_id));
		MsgGS2DS_RequireUserDataRequest user_data_rsq;
		user_data_rsq.set_account(request.account());
		if (!user_data_rsq.SerializeToArray(tmp_, sizeof(tmp_))) {
			ServerLogError("serialize msg MsgGS2DS_RequireUserDataRequest failed");
			return -1;
		}
		if (gate_conn_->send(MSGID_GS2DS_REQUIRE_USER_DATA_REQUEST, tmp_, user_data_rsq.ByteSize()) < 0) {
			ServerLogError("send msg MsgGS2DS_RequireUserDataRequest failed");
			return -1;
		}
	}
	
	ServerLogInfo("processEnterGame");
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
