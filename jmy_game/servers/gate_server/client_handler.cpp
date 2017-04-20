#include "client_handler.h"
#include "../libjmy/jmy.h"
#include "../common/util.h"
#include "../../proto/src/common.pb.h"
#include "gate_server.h"
#include "game_handler.h"
#include "global_data.h"
#include "client_manager.h"

char ClientHandler::tmp_[JMY_MAX_MSG_SIZE];
char ClientHandler::session_buf_[RECONN_SESSION_CODE_BUF_LENGTH+1];

int ClientHandler::onConnect(JmyEventInfo* info)
{
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) return -1;
	LogInfo("new client connection(%d)", info->conn_id);
	return 0;
}

int ClientHandler::onDisconnect(JmyEventInfo* info)
{
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) return -1;
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

int ClientHandler::processEnterGameRequest(JmyMsgInfo* info)
{
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) {
		LogError("get connection by info(conn_id:%d) failed", info->conn_id);
		return -1;
	}

	MsgC2S_EnterGameRequest request;
	if (!request.ParseFromArray(info->data, info->len)) {
		send_error(conn, PROTO_ERROR_LOGIN_DATA_INVALID);
		LogError("parse MsgC2S_EnterGameRequest failed");
		return -1;
	}

	ClientInfo* ci = CLIENT_MANAGER->getClientInfoByAccount(request.account());
	if (!ci) {
		send_error(conn, PROTO_ERROR_ENTER_GAME_INVALID_ACCOUNT);
		LogError("cant get ClientInfo by account(%s)", request.account().c_str());
		return -1;
	}

	// check enter session
	if (ci->enter_session != request.session_code()) {
		send_error(conn, PROTO_ERROR_LOGIN_ACCOUNT_OR_PASSWORD_INVALID);
		LogError("account %s enter session %s invalid, valid session: %s",
				request.account().c_str(), request.session_code().c_str(), ci->enter_session.c_str());
		return -1;
	}

	// check max connection
	int used_size = CLIENT_MANAGER->getUsedSize();
	if (GATE_SERVER->checkClientMaxCount(used_size)) {
		send_error(conn, PROTO_ERROR_ENTER_GAME_PLAYER_COUNT_MAXIMUM);
		// close this client
		ci->close();
		LogError("account %s enter game failed, player count(%d) is maximumu", used_size);
		return -1;
	}

	char* reconn_session = get_session_code(session_buf_, RECONN_SESSION_CODE_BUF_LENGTH);
	ci->reconn_session = reconn_session;
	ci->conn = get_connection(info);
	CLIENT_MANAGER->insertConnIdId(info->conn_id, ci->id);

	SEND_GAME_MSG(ci->id, MSGID_C2S_ENTER_GAME_REQUEST, info->data, info->len);

	return info->len;
}

int ClientHandler::processLeaveGameRequest(JmyMsgInfo* info)
{
	if (!CLIENT_MANAGER->removeByConnId(info->conn_id))
		return -1;
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
		client_info->conn = conn;
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
	return info->len;
}

int ClientHandler::processDefault(JmyMsgInfo* info)
{
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) return -1;
	int id = CLIENT_MANAGER->getIdByConnId(info->conn_id);
	if (!id) {
		send_error(conn, PROTO_ERROR_SERVER_INTERNAL_ERROR);
		LogError("not found id by conn_id(%d)", info->conn_id);
		return -1;
	}
	if (SEND_GAME_MSG(id, info->msg_id, info->data, info->len) < 0) {
		send_error(conn, PROTO_ERROR_SERVER_INTERNAL_ERROR);
		LogError("send user(%d) message(%d) to game_server(%d) failed", id, info->msg_id, GAME_SERVER_ID);
	}
	LogInfo("send user(%d) message(%d) to game_server(%d)", id, info->msg_id, GAME_SERVER_ID);
	return info->len;
}
