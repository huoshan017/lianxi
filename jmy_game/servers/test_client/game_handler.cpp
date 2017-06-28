#include "game_handler.h"
#include "../libjmy/jmy_datatype.h"
#include "../libjmy/jmy_tcp_connection.h"
#include "../common/util.h"
#include "../../proto/src/common.pb.h"
#include "config_loader.h"
#include "test_client.h"

char GameHandler::tmp_[JMY_MAX_MSG_SIZE];

int GameHandler::onConnect(JmyEventInfo* info)
{
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) {
		LogError("get connection failed in onConnect");
		return -1;
	}

	MsgC2S_GetRoleRequest request;
	std::string account;
	if (!CLIENT_MGR->getAccountByConnId(info->conn_id, account)) {
		LogError("get account by conn_id(%d) failed", info->conn_id);
		return -1;
	}
	request.set_account(account);
	TestClient* client = CLIENT_MGR->getClientByConnId(info->conn_id);
	if (!client) {
		LogError("get TestClient failed by conn_id(%d)", info->conn_id);
		return -1;
	}
	request.set_enter_session(client->getEnterSession());
	if (!request.SerializeToArray(tmp_, sizeof(tmp_))) {
		LogError("serialize msg MsgC2S_EnterGameRequest failed");
		return -1;
	}
	if (conn->send(MSGID_C2S_GET_ROLE_REQUEST, tmp_, request.ByteSize()) < 0) {
		LogError("send msg MsgC2S_EnterGameRequest failed");
		return -1;
	}

	static int connect_count = 0;
	LogInfo("game onConnect, connect_count(%d)", ++connect_count);
	return 0;
}

int GameHandler::onDisconnect(JmyEventInfo* info)
{
	(void)info;
	static int disconnect_count = 0;
	LogInfo("game onDisconnect, disconnect_count(%d)", ++disconnect_count);
	return 0;
}

int GameHandler::onTick(JmyEventInfo* info)
{
	(void)info;
	return 0;
}

int GameHandler::processGetRole(JmyMsgInfo* info)
{
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) return -1;
	MsgS2C_GetRoleResponse response;
	if (!response.ParseFromArray(info->data, info->len)) {
		LogError("parse MsgS2C_GetRoleResponse failed");
		return -1;
	}

	const MsgBaseRoleData& rd = response.role_data();
	LogInfo("get role: race(%d), sex(%d), role_id(%llu), nick_name(%s)", rd.race(), rd.sex(), rd.role_id(), rd.nick_name().c_str());

	TestClient* client = CLIENT_MGR->getClientByConnId(info->conn_id);
	if (!client) {
		LogError("get TestClient failed by conn_id(%d)", info->conn_id);
		return -1;
	}

	client->setReconnSession(response.reconnect_session());

	if (send_enter_game_request(conn) < 0)
		return -1;

	static int get_count = 0;
	LogInfo("get_count(%d)", ++get_count);

	return info->len;
}

int GameHandler::processCreateRole(JmyMsgInfo* info)
{
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) return -1;
	MsgS2C_CreateRoleResponse response;
	if (!response.ParseFromArray(info->data, info->len)) {
		LogError("parse MsgS2C_CreateRoleResponse failed");
		return -1;
	}

	const MsgBaseRoleData& rd = response.role_data();
	LogInfo("create role: race(%d), sex(%d), role_id(%llu), nick_name(%s)", rd.race(), rd.sex(), rd.role_id(), rd.nick_name().c_str());

	if (send_enter_game_request(conn) < 0)
		return -1;

	static int create_count = 0;
	LogInfo("create_count(%d)", ++create_count);
	return info->len;
}

int GameHandler::processEnterGame(JmyMsgInfo* info)
{
	MsgS2C_EnterGameResponse response;
	if (!response.ParseFromArray(info->data, info->len)) {
		LogError("parse MsgS2C_EnterGameResponse failed");
		return -1;
	}

	std::string account;
	if (!CLIENT_MGR->getAccountByConnId(info->conn_id, account)) {
		LogError("cant get account by conn_id(%d)", info->conn_id);
		return -1;
	}

	static int enter_count = 0;
	LogInfo("%s enter game, enter_count(%d)", account.c_str(), ++enter_count);
	return info->len;
}

int GameHandler::processEnterGameComplete(JmyMsgInfo* info)
{
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) return -1;

#if 0
	MsgC2S_SetRoleDataRequest request;
	for (int i=0; i<1; ++i) {
		MsgBaseRoleData* role_data = request.mutable_role_data();
		role_data->set_hp(1+i);
		role_data->set_sex(1+i);
		role_data->set_race(1+i);
		role_data->set_level(2+i);
		if (!request.SerializeToArray(tmp_, sizeof(tmp_))) {
			LogError("serialize MsgC2S_SetRoleDataRequest failed");
			return -1;
		}

		if (conn->send(MSGID_C2S_SET_ROLE_DATA_REQUEST, tmp_, request.ByteSize()) < 0) {
			LogError("send MsgC2S_SetRoleDataRequest failed");
			return -1;
		}
	}

	MsgC2S_ChatRequest chat_req;
	chat_req.set_content("/gm additem 10000 1");
	if (!chat_req.SerializeToArray(tmp_, sizeof(tmp_))) {
		LogError("serialize MsgC2S_ChatRequest failed");
		return -1;
	}
	if (conn->send(MSGID_C2S_CHAT_REQUEST, tmp_, chat_req.ByteSize()) < 0) {
		LogError("send MsgC2S_ChatRequest failed");
		return -1;
	}
#endif

	LogInfo("enter game complete");
	return info->len;
}

int GameHandler::processReconnect(JmyMsgInfo* info)
{
	return info->len;
}

int GameHandler::processError(JmyMsgInfo* info)
{
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) return -1;
	MsgError error;
	if (!error.ParseFromArray(info->data, info->len)) {
		LogError("parse MsgError failed");
		return -1;
	}

	if (error.error_code() == PROTO_ERROR_GET_ROLE_NONE) {
#if 1
		MsgC2S_CreateRoleRequest request;
		request.set_sex(0);
		request.set_race(0);
		request.set_nick_name("huoshan017");
		if (!request.SerializeToArray(tmp_, sizeof(tmp_))) {
			LogError("serialize MsgC2S_CreateRoleRequest failed");
			return -1;
		}
		if (conn->send(MSGID_C2S_CREATE_ROLE_REQUEST, tmp_, request.ByteSize()) < 0) {
			LogError("send MsgC2S_CreateRoleRequest failed");
			return -1;
		}
		LogInfo("send create role request");
#endif
		std::string account;
		if (!CLIENT_MGR->getAccountByConnId(info->conn_id, account)) {
			LogError("get account failed by conn_id(%d)", info->conn_id);
			return -1;
		}
		static int err_count = 0;
		err_count += 1;
		LogInfo("account(%s) err_count(%d)", account.c_str(), err_count);
	}

	LogInfo("processError");
	return info->len;
}

int GameHandler::processDefault(JmyMsgInfo* info)
{
	return info->len;
}

int GameHandler::send_enter_game_request(JmyTcpConnection* conn)
{
	MsgC2S_EnterGameRequest enter_req;
	if (!enter_req.SerializeToArray(tmp_, sizeof(tmp_))) {
		LogError("serialize MsgC2S_EnterGameRequest failed");
		return -1;
	}
	int res = conn->send(MSGID_C2S_ENTER_GAME_REQUEST, tmp_, enter_req.ByteSize());
	if (res < 0) {
		LogError("send MsgC2S_EnterGameRequest failed");
		return -1;
	}
	return res;
}
