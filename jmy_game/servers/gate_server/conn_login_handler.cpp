#include "conn_login_handler.h"
#include "../libjmy/jmy_mem.h"
#include "../libjmy/jmy_const.h"
#include "../common/util.h"
#include "../../proto/src/server.pb.h"
#include "client_handler.h"
#include "config_loader.h"


char ConnLoginHandler::tmp_[JMY_MAX_MSG_SIZE];
LoginAgentManager ConnLoginHandler::login_mgr_;
char ConnLoginHandler::session_code_buff_[ENTER_GAME_SESSION_CODE_LENGTH+1];

int ConnLoginHandler::onConnect(JmyEventInfo* info)
{
	MsgGT2LS_ConnectLoginRequest request;
	request.set_gate_server_id(CONFIG_FILE.id);
	request.set_gate_server_ip(CONFIG_FILE.ip.c_str());
	request.set_gate_server_port(CONFIG_FILE.port);
	if (!request.SerializeToArray(tmp_, sizeof(tmp_))) {
		ServerLogError("serialize MsgGT2LS_ConnectLoginRequest failed");
		return -1;
	}

	JmyTcpConnection* conn = get_connection(info);
	if (!conn) return -1;
	if (conn->send(MSGID_GT2LS_CONNECT_LOGIN_REQUEST, tmp_, request.ByteSize()) < 0) {
		ServerLogError("send MsgGT2LS_ConnectLoginRequest failed");
		return -1;
	}

	ServerLogInfo("onconnect to login_server");
	return 0;
}

int ConnLoginHandler::onDisconnect(JmyEventInfo* info)
{
	LoginAgent* agent = login_mgr_.getAgentByConnId(info->conn_id);
	if (!agent) {
		ServerLogError("not found login agent with conn_id(%d)", info->conn_id);
		return -1;
	}
	login_mgr_.deleteAgent(agent->getId());
	ServerLogInfo("login agent with conn_id(%d) disconnected", info->conn_id);
	return 0;
}

int ConnLoginHandler::onTick(JmyEventInfo* info)
{
	(void)info;
	return 0;
}

int ConnLoginHandler::onTimer(JmyEventInfo* info)
{
	(void)info;
	return 0;
}

// process connect login server response
int ConnLoginHandler::processConnectLoginResponse(JmyMsgInfo* info)
{
	MsgLS2GT_ConnectLoginResponse response;
	if (!response.ParseFromArray(info->data, info->len)) {
		ServerLogError("parse MsgLS2GT_ConnectLoginResponse failed");
		return -1;
	}
	int login_id = response.login_id();
	if (get_server_type(login_id) != SERVER_TYPE_LOGIN) {
		ServerLogError("login_id(%d) is not login server_type", login_id);
		return -1;
	}
	LoginAgent* agent = login_mgr_.getAgent(login_id);
	if (agent) {
		ServerLogError("already exist login agent for login_id(%d)", login_id);
		return -1;
	}
	agent = login_mgr_.newAgent(login_id, (JmyTcpConnectionMgr*)info->param, info->session_id);
	if (!agent) {
		ServerLogError("create new login agent failed with login_id(%d) and conn_id(%d)", login_id, info->session_id);
		return -1;
	}
	ServerLogInfo("processed connect login server %d response", login_id);
	return 0;
}

// process selected gate server notify
int ConnLoginHandler::processSelectedServerNotify(JmyMsgInfo* info)
{
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) {
		ServerLogError("login connection with id(%d) not found", info->session_id);
		return -1;
	}

	MsgLS2GT_SelectedServerNotify notify;
	notify.ParseFromArray(info->data, info->len);
	// generate session code to verify new client
	char* session_code = get_session_code(session_code_buff_, ENTER_GAME_SESSION_CODE_LENGTH);
	if (!ClientHandler::newClientSession(notify.account().c_str(), session_code)) {
		ServerLogError("create new account(%s) and session(%s) failed", notify.account().c_str(), session_code);
		return -1;
	}

	MsgGT2LS_SelectedServerResponse response;
	response.set_account(notify.account());
	response.set_session_code(session_code);
	response.SerializeToArray(tmp_, sizeof(tmp_));
	conn->send(MSGID_GT2LS_SELECTED_SERVER_RESPONSE, tmp_, response.ByteSize());
	return 0;
}

