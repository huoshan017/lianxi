#include "client_handler.h"
#include "../libjmy/jmy.h"
#include "../common/util.h"
#include "../../proto/src/common.pb.h"
#include "gate_server.h"
#include "game_handler.h"

char ClientHandler::tmp_[JMY_MAX_MSG_SIZE];
char ClientHandler::session_buf_[RECONN_SESSION_CODE_BUF_LENGTH+1];
BiMap<std::string, int> ClientHandler::account_id_bimap_;
BiMap<int, int> ClientHandler::connid_id_bimap_;
ClientArray ClientHandler::client_array_;

bool ClientHandler::init()
{
	if (!client_array_.init(CONFIG_FILE.max_conn)) {
		return false;
	}
	return true;
}

void ClientHandler::clear()
{
	client_array_.clear();
}

int ClientHandler::onConnect(JmyEventInfo* info)
{
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) return -1;
	ServerLogInfo("new client connection(%d)", info->conn_id);
	return 0;
}

int ClientHandler::onDisconnect(JmyEventInfo* info)
{
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) return -1;
	connid_id_bimap_.remove_1(info->conn_id);
	ServerLogInfo("client connection(%d) disconnect", info->conn_id);
	return 0;
}

int ClientHandler::onTick(JmyEventInfo* info)
{
	(void)info;
	return 0;
}

int ClientHandler::onTimer(JmyEventInfo* info)
{
	(void)info;
	return 0;
}

bool ClientHandler::newClientSession(const std::string& account, const std::string& session_code)
{
	int id = 0;
	bool b = account_id_bimap_.find_1(account, id);
	if (b) {
		ClientInfo* info = client_array_.get(id);
		if (!info) {
			ServerLogError("cant found ClientInfo by account:%s id:%d", account.c_str(), id);
			return false;
		}
		info->account = account;
		info->enter_session = session_code;
	} else {
		ClientInfo* info = client_array_.getFree();
		if (!info) {
			ServerLogError("cant get free ClientInfo for account(%s) to hold enter_session(%s)", account.c_str(), session_code.c_str());
			return false;
		}
		account_id_bimap_.insert(account, info->id);
	}
	return true;
}

void ClientHandler::send_error(JmyTcpConnection* conn, ProtoErrorType error)
{
	if (!conn) return;
	MsgError response;
	response.set_error_code(error);
	response.SerializeToArray(tmp_, sizeof(tmp_));
	conn->send(MSGID_ERROR, tmp_, response.ByteSize());
}

int ClientHandler::getClientStartId()
{
	return client_array_.getStartId();
}

ClientInfo* ClientHandler::getClientInfo(int user_id)
{
	return client_array_.get(user_id);
}

ClientInfo* ClientHandler::getClientInfoByAccount(const std::string& account)
{
	int id = 0;
	bool b = account_id_bimap_.find_1(account, id);
	if (!b) {
		ServerLogError("account(%s) not found in account_id_map_", account.c_str());
		return nullptr;
	}
	return client_array_.get(id);
}

ClientInfo* ClientHandler::getClientInfoByConnId(int conn_id)
{
	int id = 0;
	bool b = connid_id_bimap_.find_1(conn_id, id);
	if (!b) {
		ServerLogError("conn_id(%d) not found in connid_id_map_", conn_id);
		return nullptr;
	}

	return client_array_.get(id);
}

int ClientHandler::processEnterGame(JmyMsgInfo* info)
{
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) {
		ServerLogError("get connection by info(conn_id:%d) failed", info->session_id);
		return -1;
	}

	MsgC2S_EnterGameRequest request;
	if (!request.ParseFromArray(info->data, info->len)) {
		send_error(conn, PROTO_ERROR_LOGIN_DATA_INVALID);
		ServerLogError("parse MsgC2S_EnterGameRequest failed");
		return -1;
	}

	ClientInfo* ci = getClientInfoByAccount(request.account());
	if (!ci) {
		send_error(conn, PROTO_ERROR_ENTER_GAME_INVALID_ACCOUNT);
		ServerLogError("cant get ClientInfo by account(%s)", request.account().c_str());
		return -1;
	}

	// check enter session
	if (ci->enter_session != request.session_code()) {
		send_error(conn, PROTO_ERROR_LOGIN_ACCOUNT_OR_PASSWORD_INVALID);
		ServerLogError("account %s enter session %s invalid, valid session: %s",
				request.account().c_str(), request.session_code().c_str(), ci->enter_session.c_str());
		return -1;
	}

	// check max connection
	if (GATE_SERVER->checkClientMaxCount(client_array_.getUsedSize())) {
		send_error(conn, PROTO_ERROR_ENTER_GAME_PLAYER_COUNT_MAXIMUM);
		ci->close();
		ServerLogError("account %s enter game failed, player count(%d) is maximumu", client_array_.getUsedSize());
		return -1;
	}

	char* reconn_session = get_session_code(session_buf_, RECONN_SESSION_CODE_BUF_LENGTH);
	ci->reconn_session = reconn_session;
	ci->conn = get_connection(info);
	connid_id_bimap_.insert(info->session_id, ci->id);

	SEND_GAME_MSG(ci->id, MSGID_C2S_ENTER_GAME_REQUEST, info->data, info->len);

	return info->len;
}

int ClientHandler::processLeaveGame(JmyMsgInfo* info)
{
	connid_id_bimap_.remove_1(info->session_id);
	return info->len;
}

int ClientHandler::processReconnect(JmyMsgInfo* info)
{
	MsgC2S_ReconnectRequest request;
	if (!request.ParseFromArray(info->data, info->len)) {
		ServerLogError("parse MsgCL2GT_ReconnectRequest failed");
		return -1;
	}

	JmyTcpConnection* conn = get_connection(info);
	if (!conn) {
		ServerLogError("cant get connection by info(conn_id:%d)", info->session_id);
		return -1;
	}

	ClientInfo* client_info = getClientInfoByAccount(request.account());
	if (!client_info) {
		send_error(conn, PROTO_ERROR_RECONN_ACCOUNT_NOT_FOUND);
		ServerLogError("account(%s) reconnect must has ClientInfo, but not found", request.account().c_str());
		return -1;
	}

	// check connection is the same
	if (!client_info->check_conn(conn)) {
		client_info->force_close();
		client_info->conn = conn;
		ServerLogInfo("exist account %s, kick it", request.account().c_str());
	}

	// resend data
	MsgS2C_ReconnectResponse response;
	char* reconn_session = get_session_code(session_buf_, RECONN_SESSION_CODE_BUF_LENGTH);
	client_info->reconn_session = reconn_session;
	response.set_reconnect_session(client_info->reconn_session);
	if (!response.SerializeToArray(tmp_, sizeof(tmp_))) {
		send_error(conn, PROTO_ERROR_RECONN_FAILED);
		ServerLogError("serialize MsgS2C_ReconnectResponse failed");
		return -1;
	}
	conn->send(MSGID_S2C_RECONNECT_RESPONSE, tmp_, response.ByteSize());
	return info->len;
}

int ClientHandler::processDefault(JmyMsgInfo* info)
{
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) return -1;
	int id = 0;
	if (!connid_id_bimap_.find_1(info->session_id, id)) {
		send_error(conn, PROTO_ERROR_SERVER_INTERNAL_ERROR);
		ServerLogError("not found id by conn_id(%d)", info->session_id);
		return -1;
	}
	if (SEND_GAME_MSG(id, info->msg_id, info->data, info->len) < 0) {
		send_error(conn, PROTO_ERROR_SERVER_INTERNAL_ERROR);
		ServerLogError("send user(%d) message(%d) to game_server(%d) failed", id, info->msg_id, GAME_SERVER_ID);
	}
	ServerLogInfo("send user(%d) message(%d) to game_server(%d)", id, info->msg_id, GAME_SERVER_ID);
	return info->len;
}

int ClientHandler::sendEnterGameResponse2Client(int id)
{
	ClientInfo* ci = client_array_.get(id);
	if (!ci) {
		ServerLogError("get ClientInfo by id(%d) failed", id);
		return -1;
	}
	MsgS2C_EnterGameResponse response;
	response.set_reconnect_session(ci->reconn_session.c_str());
	if (!response.SerializeToArray(tmp_, sizeof(tmp_))) {
		ServerLogError("serialize MsgS2C_EnterGameResponse failed");
		return -1;
	}
	int res = ci->send(MSGID_S2C_ENTER_GAME_RESPONSE, tmp_, response.ByteSize());
	if (res < 0) {
		ServerLogError("send message MsgS2C_EnterGameResponse failed to client(%d) failed", id);
		return -1;
	}
	ServerLogInfo("send message MsgS2C_EnterGameResponse to client(%d)", id);
	return res;
}
