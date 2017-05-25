#include "game_handler.h"
#include "../libjmy/jmy_datatype.h"
#include "../libjmy/jmy_tcp_connection.h"
#include "../common/util.h"
#include "../../proto/src/common.pb.h"
#include "config_loader.h"

char GameHandler::tmp_[JMY_MAX_MSG_SIZE];
std::string GameHandler::enter_session_;
std::string GameHandler::reconn_session_;

int GameHandler::onConnect(JmyEventInfo* info)
{
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) {
		LogError("get connection failed in onConnect");
		return -1;
	}

	MsgC2S_GetRoleRequest request;
	request.set_enter_session(enter_session_);
	request.set_account(CLIENT_CONFIG.account);
	if (!request.SerializeToArray(tmp_, sizeof(tmp_))) {
		LogError("serialize msg MsgC2S_EnterGameRequest failed");
		return -1;
	}
	if (conn->send(MSGID_C2S_GET_ROLE_REQUEST, tmp_, request.ByteSize()) < 0) {
		LogError("send msg MsgC2S_EnterGameRequest failed");
		return -1;
	}
	LogInfo("game onConnect");
	return 0;
}

int GameHandler::onDisconnect(JmyEventInfo* info)
{
	(void)info;
	LogInfo("game onDisconnect");
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

	reconn_session_ = response.reconnect_session();
	const MsgBaseRoleData& role_data = response.role_data();
	if (send_enter_game_request(conn, role_data.role_id()) < 0)
		return -1;

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

	const MsgBaseRoleData& role_data = response.role_data();
	if (send_enter_game_request(conn, role_data.role_id()) < 0)
		return -1;

	return info->len;
}

int GameHandler::processEnterGame(JmyMsgInfo* info)
{
	MsgS2C_EnterGameResponse response;
	if (!response.ParseFromArray(info->data, info->len)) {
		LogError("parse MsgS2C_EnterGameResponse failed");
		return -1;
	}
	LogInfo("processEnterGame");
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
	}

	LogInfo("processError");
	return info->len;
}

int GameHandler::processDefault(JmyMsgInfo* info)
{
	return info->len;
}

void GameHandler::setEnterSession(const std::string& session)
{
	enter_session_ = std::move(session);
}

int GameHandler::send_enter_game_request(JmyTcpConnection* conn, uint64_t role_id)
{
	MsgC2S_EnterGameRequest enter_req;
	enter_req.set_role_id(role_id);
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
