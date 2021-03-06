#include "client_handler.h"
#include "../libjmy/jmy.h"
#include "../common/util.h"
#include "../../proto/src/msgid.pb.h"
#include "../../proto/src/account.pb.h"
#include "../../proto/src/server.pb.h"
#include "gate_server.h"
#include "game_handler.h"
#include "global_data.h"
#include "client_manager.h"

char ClientHandler::tmp_[JMY_MAX_MSG_SIZE];
char ClientHandler::session_buf_[RECONN_SESSION_CODE_BUF_LENGTH+1];

int ClientHandler::onConnect(JmyEventInfo* info)
{
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) {
		LogError("cant get connection by info");
		return -1;
	}
	LogInfo("new client connection(%d)", info->conn_id);
	return 0;
}

int ClientHandler::onDisconnect(JmyEventInfo* info)
{
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) {
		LogError("cant get connection by info");
		return -1;
	}
	if (!CLIENT_MANAGER->removeByConnId(info->conn_id))
		return -1;
	LogInfo("client connection(%d) disconnect", info->conn_id);
	return 0;
}

int ClientHandler::onTick(JmyEventInfo* info)
{
	(void)info;
	return 0;
}

int ClientHandler::processGetRoleRequest(JmyMsgInfo* info)
{
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) {
		LogInfo("get connection failed");
		return -1;
	}

	MsgC2S_GetRoleRequest request;
	if (!request.ParseFromArray(info->data, info->len)) {
		LogError("parse MsgC2S_GetRoleRequest failed");
		return -1;
	}

	ClientInfo* ci = CLIENT_MANAGER->getClientInfoByAccount(request.account());
	if (ci) {
		ci->force_close();
		LogWarn("already exist account(%s), to kick it", request.account().c_str());
	}

	// check enter session
	if (!CLIENT_MANAGER->checkSessionByAccount(request.account(), request.enter_session())) {
		send_error(conn, PROTO_ERROR_LOGIN_ACCOUNT_OR_PASSWORD_INVALID);
		LogError("account(%s) enter session(%s) invalid", request.account().c_str(), request.enter_session().c_str());
		return -1;
	}

	ci = CLIENT_MANAGER->newClient(request.account());
	if (!ci) {
		send_error(conn, PROTO_ERROR_ENTER_GAME_INVALID_ACCOUNT);
		LogError("cant get ClientInfo by account(%s)", request.account().c_str());
		return -1;
	}

	// check max connection
	int used_size = CLIENT_MANAGER->getUsedSize();
	if (GATE_SERVER->checkClientMaxCount(used_size)) {
		send_error(conn, PROTO_ERROR_ENTER_GAME_PLAYER_COUNT_MAXIMUM);
		// close this client
		ci->close();
		LogError("account %s enter game failed, player count(%d) is maximumu", request.account().c_str(), used_size);
		return -1;
	}

	// insert mapping: conn_id <-> id
	CLIENT_MANAGER->insertConnIdId(info->conn_id, ci->id);
	ci->conn_id = info->conn_id;
	ci->conn_mgr = (JmyTcpConnectionMgr*)info->param;

	MsgGT2GS_GetRoleRequest get_req;
	get_req.set_account(ci->account);
	if (!get_req.SerializeToArray(tmp_, sizeof(tmp_))) {
		LogError("serialize MsgGT2GS_GetRoleRequest failed");
		return -1;
	}
	if (SEND_GAME_MSG(MSGID_GT2GS_GET_ROLE_REQUEST, info->data, info->len) < 0) {
		LogError("send MsgGT2GS_GetRoleRequest failed");
		return -1;
	}

	static int get_count = 0;
	LogInfo("process get role request: account(%s), get_count(%d)", ci->account.c_str(), ++get_count);

	return info->len;
}

int ClientHandler::processCreateRoleRequest(JmyMsgInfo* info)
{
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) {
		LogError("get connection failed by info");
		return -1;
	}

	ClientInfo* ci = CLIENT_MANAGER->getClientInfoByConnId(info->conn_id);
	if (!ci) {
		LogError("cant get ClientInfo by conn_id(%d)", info->conn_id);
		return -1;
	}

	MsgC2S_CreateRoleRequest request;
	if (!request.ParseFromArray(info->data, info->len)) {
		LogError("parse MsgC2S_CreateRoleRequest failed");
		return -1;
	}

	MsgGT2GS_CreateRoleRequest create_req;
	create_req.set_account(ci->account);
	create_req.set_sex(request.sex());
	create_req.set_race(request.race());
	create_req.set_nick_name(request.nick_name());
	if (!create_req.SerializeToArray(tmp_, sizeof(tmp_))) {
		LogError("serialize MsgGT2GS_CreateRoleRequest failed");
		return -1;
	}

	SEND_GAME_MSG(MSGID_GT2GS_CREATE_ROLE_REQUEST, tmp_, create_req.ByteSize());

	static int create_count = 0;
	LogInfo("process create role request: account(%s), count(%d)", ci->account.c_str(), ++create_count);

	return info->len;
}

int ClientHandler::processEnterGameRequest(JmyMsgInfo* info)
{
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) {
		LogError("get connection by info(conn_id:%d) failed", info->conn_id);
		return -1;
	}

	// check role id is valid
	ClientInfo* ci = CLIENT_MANAGER->getClientInfoByConnId(info->conn_id);
	if (!ci) {
		LogError("not found ClientInfo by conn_id(%d)", info->conn_id);
		return -1;
	}

	if (ci->curr_role_id == 0) {
		send_error(conn, PROTO_ERROR_ENTER_GAME_INVALID_ACCOUNT);
		LogError("current role_id not found");
		return -1;
	}

	MsgGT2GS_EnterGameRequest request;
	request.set_role_id(ci->curr_role_id);
	if (!request.SerializeToArray(tmp_, sizeof(tmp_))) {
		LogError("serialize MsgGT2GS_EnterGameRequest failed");
		return -1;
	}

	SEND_GAME_USER_MSG(ci->id, MSGID_GT2GS_ENTER_GAME_REQUEST, tmp_, request.ByteSize());

	static int enter_count = 0;
	LogInfo("process enter game request, role_id(%llu), enter_count(%d)", ci->curr_role_id, ++enter_count);

	return info->len;
}

int ClientHandler::processLeaveGameRequest(JmyMsgInfo* info)
{
	ClientInfo* ci = CLIENT_MANAGER->getClientInfoByConnId(info->conn_id);
	if (!ci) return -1;
	ci->close();

	MsgGT2GS_LeaveGameRequest request;
	if (!request.SerializeToArray(tmp_, sizeof(tmp_))) {
		LogError("serialize MsgGT2GS_LeaveGameRequest failed");
		return -1;
	}

	if (SEND_GAME_USER_MSG(ci->id, MSGID_GT2GS_LEAVE_GAME_REQUEST, tmp_, request.ByteSize()) < 0) {
		LogError("send MsgGT2GS_LeaveGameRequest failed");
		return -1;
	}

	LogInfo("process leave game request, role_id(%llu)", ci->curr_role_id);
	return info->len;
}

int ClientHandler::processReconnectRequest(JmyMsgInfo* info)
{
	MsgC2S_ReconnectRequest request;
	if (!request.ParseFromArray(info->data, info->len)) {
		LogError("parse MsgCL2GT_ReconnectRequest failed");
		return -1;
	}

	JmyTcpConnection* conn = get_connection(info);
	if (!conn) {
		LogError("cant get connection by info(conn_id:%d)", info->conn_id);
		return -1;
	}

	ClientInfo* client_info = CLIENT_MANAGER->getClientInfoByAccount(request.account());
	if (!client_info) {
		send_error(conn, PROTO_ERROR_RECONN_ACCOUNT_NOT_FOUND);
		LogError("account(%s) reconnect must has ClientInfo, but not found", request.account().c_str());
		return -1;
	}

	// check connection is the same
	if (!client_info->check_conn(conn)) {
		client_info->force_close();
		LogInfo("exist account %s, kick it", request.account().c_str());
	}

	// resend data
	MsgS2C_ReconnectResponse response;
	char* reconn_session = get_session_code(session_buf_, RECONN_SESSION_CODE_BUF_LENGTH);
	client_info->reconn_session = reconn_session;
	response.set_reconnect_session(client_info->reconn_session);
	if (!response.SerializeToArray(tmp_, sizeof(tmp_))) {
		send_error(conn, PROTO_ERROR_RECONN_FAILED);
		LogError("serialize MsgS2C_ReconnectResponse failed");
		return -1;
	}
	if (conn->send(MSGID_S2C_RECONNECT_RESPONSE, tmp_, response.ByteSize()) < 0) {
		LogError("send MsgS2C_ReconnectResponse failed");
		return -1;
	}

	LogInfo("process reconnect request: account(%s)", request.account().c_str());
	return info->len;
}

int ClientHandler::processDefault(JmyMsgInfo* info)
{
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) {
		LogError("get connection failed by info");
		return -1;
	}
	int id = CLIENT_MANAGER->getIdByConnId(info->conn_id);
	if (!id) {
		send_error(conn, PROTO_ERROR_SERVER_INTERNAL_ERROR);
		LogError("not found id by conn_id(%d)", info->conn_id);
		return -1;
	}
	if (SEND_GAME_USER_MSG(id, info->msg_id, info->data, info->len) < 0) {
		send_error(conn, PROTO_ERROR_SERVER_INTERNAL_ERROR);
		LogError("send user(%d) message(%d) to game_server(%d) failed", id, info->msg_id, GAME_SERVER_ID);
	}
	LogInfo("send user(%d) message(%d) to game_server(%d)", id, info->msg_id, GAME_SERVER_ID);
	return info->len;
}
