#include "conn_gate_handler.h"
#include "../libjmy/jmy_datatype.h"
#include "../libjmy/jmy_tcp_connection.h"
#include "../common/util.h"
#include "../../proto/src/server.pb.h"
#include "config_loader.h"
#include "player.h"
#include "global_data.h"

char ConnGateHandler::tmp_[JMY_MAX_MSG_SIZE];

int ConnGateHandler::onConnect(JmyEventInfo* info)
{
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) return -1;
	MsgGS2GT_ConnectGateRequest request;
	request.set_game_id(SERVER_CONFIG.id);
	if (!request.SerializeToArray(tmp_, sizeof(tmp_))) {
		LogError("serialize MsgGS2GT_ConnectGateRequest failed");
		return -1;
	}
	if (conn->send(MSGID_GS2GT_CONNECT_GATE_REQUEST, tmp_, request.ByteSize()) < 0) {
		LogError("send message MsgGS2GT_ConnectGateRequest failed");
		return -1;
	}
	LogInfo("send message MsgGS2GT_ConnectGateRequest to gate server");
	return 0;
}

int ConnGateHandler::onDisconnect(JmyEventInfo* info)
{
	(void)info;
	GLOBAL_DATA->setGateConn(nullptr);
	LogInfo("ondisconnect");
	return 0;
}

int ConnGateHandler::onTick(JmyEventInfo* info)
{
	(void)info;
	return 0;
}

int ConnGateHandler::processConnectGateResponse(JmyMsgInfo* info)
{
	if (PLAYER_MGR->isInited()) {
		LogWarn("already connected gate server");
		return info->len;
	}

	JmyTcpConnection* conn = get_connection(info);
	if (!conn) {
		LogError("cant get connection");
		return -1;
	}

	MsgGT2GS_ConnectGateResponse response;
	if (!response.ParseFromArray(info->data, info->len)) {
		LogError("parse MsgGT2GS_ConnectGateResponse failed");
		return -1;
	}

	if (!PLAYER_MGR->init(response.start_user_id(), response.max_user_count())) {
		LogError("PlayerManager init with start_user_id(%d) max_user_count(%d) failed",
				response.start_user_id(), response.max_user_count());
		return -1;
	}

	GLOBAL_DATA->setGateConn(conn);

	LogInfo("processConnectGateResponse: start_user_id(%d), max_user_count(%d)",
			response.start_user_id(), response.max_user_count());
	
	return info->len;
}

int ConnGateHandler::processEnterGame(JmyMsgInfo* info)
{
	MsgGT2GS_EnterGameRequest request;
	if (!request.ParseFromArray(info->data, info->len)) {
		LogError("parse MsgGT2GS_EnterGameRequest failed");
		return -1;
	}

	int user_id = info->user_id;
	Player* p = PLAYER_MGR->get(user_id);
	// player not found, send message to db_server
	if (!p) {
		PLAYER_MGR->addAccountId(request.account(), user_id);
		MsgGS2DS_RequireUserDataRequest user_data_req;
		user_data_req.set_account(request.account());
		if (!user_data_req.SerializeToArray(tmp_, sizeof(tmp_))) {
			LogError("serialize msg MsgGS2DS_RequireUserDataRequest failed");
			return -1;
		}
		if (GLOBAL_DATA->sendDB(MSGID_GS2DS_REQUIRE_USER_DATA_REQUEST, tmp_, user_data_req.ByteSize()) < 0) {
			LogError("send msg MsgGS2DS_RequireUserDataRequest failed");
			return -1;
		}
		LogInfo("send MsgGS2DS_RequireUserDataRequest to db_server");
	} else {
		MsgGS2GT_EnterGameResponse response;
		if (response.SerializeToArray(tmp_, sizeof(tmp_))) {
			LogError("serialize MsgGS2GT_EnterGameResponse failed");
			return -1;
		}
		if (GLOBAL_DATA->sendGate(user_id, MSGID_GS2GT_ENTER_GAME_RESPONSE, tmp_, response.ByteSize()) < 0) {
			LogError("send MsgGS2GT_EnterGameResponse failed");
			return -1;
		}
		LogInfo("send MsgGS2GT_EnterGameResponse to gate_server");
	}
	
	LogInfo("processEnterGame: user_id(%d)", user_id);
	return info->len;
}

int ConnGateHandler::processLeaveGame(JmyMsgInfo* info)
{
	MsgGT2GS_LeaveGameRequest request;
	if (!request.ParseFromArray(info->data, info->len)) {
		LogError("parse MsgGT2GS_LeaveGameRequest failed");
		return -1;
	}

	Player* p = PLAYER_MGR->get(info->user_id);
	if (!p) {
		LogError("cant get Player by user_id(%d)", info->user_id);
		return -1;
	}
	
	LogInfo("processLeaveGame: user_id(%d)", info->user_id);
	return info->len;
}

int ConnGateHandler::processDefault(JmyMsgInfo* info)
{
	switch (info->msg_id) {
	}
	return info->len;
}
