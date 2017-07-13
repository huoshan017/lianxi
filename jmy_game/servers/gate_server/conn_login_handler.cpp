#include "conn_login_handler.h"
#include "../libjmy/jmy_mem.h"
#include "../libjmy/jmy_const.h"
#include "../common/util.h"
#include "../../proto/src/server.pb.h"
#include "client_handler.h"
#include "config_loader.h"
#include "global_data.h"
#include "client_manager.h"

char ConnLoginHandler::tmp_[JMY_MAX_MSG_SIZE];
char ConnLoginHandler::session_code_buff_[ENTER_GAME_SESSION_CODE_LENGTH+1];

int ConnLoginHandler::onConnect(JmyEventInfo* info)
{
	MsgGT2LS_ConnectLoginRequest request;
	request.set_gate_server_id(CONFIG_FILE.id);
	request.set_gate_server_ip(CONFIG_FILE.ip.c_str());
	request.set_gate_server_port(CONFIG_FILE.port);
	if (!request.SerializeToArray(tmp_, sizeof(tmp_))) {
		LogError("serialize MsgGT2LS_ConnectLoginRequest failed");
		return -1;
	}

	JmyTcpConnection* conn = get_connection(info);
	if (!conn) {
		LogError("get connection by info failed");
		return -1;
	}
	if (conn->send(MSGID_GT2LS_CONNECT_LOGIN_REQUEST, tmp_, request.ByteSize()) < 0) {
		LogError("send MsgGT2LS_ConnectLoginRequest failed");
		return -1;
	}

	LogInfo("onconnect to login_server");
	return 0;
}

int ConnLoginHandler::onDisconnect(JmyEventInfo* info)
{
	if (!GLOBAL_DATA->getLoginAgentByConnId(info->conn_id)) {
		LogError("not found login agent with conn_id(%d)", info->conn_id);
		return -1;
	}

	if (!GLOBAL_DATA->removeLoginAgentAndClientByConnId(info->conn_id)) {
		LogWarn("cant remove login agent and client by conn_id(%d)", info->conn_id);
		return 0;
	}

	LogInfo("login agent with conn_id(%d) disconnected", info->conn_id);
	return 0;
}

int ConnLoginHandler::onTick(JmyEventInfo* info)
{
	(void)info;
	return 0;
}

// process connect login server response
int ConnLoginHandler::processConnectLoginResponse(JmyMsgInfo* info)
{
	MsgLS2GT_ConnectLoginResponse response;
	if (!response.ParseFromArray(info->data, info->len)) {
		LogError("parse MsgLS2GT_ConnectLoginResponse failed");
		return -1;
	}
	int login_id = response.login_id();
	if (get_server_type(login_id) != SERVER_TYPE_LOGIN) {
		LogError("login_id(%d) is not login server_type", login_id);
		return -1;
	}

	if (GLOBAL_DATA->getLoginAgent(login_id)) {
		LogError("already exist login agent for login_id(%d)", login_id);
		return -1;
	}
	LoginAgent* agent = GLOBAL_DATA->newLoginAgent(login_id, (JmyTcpConnectionMgr*)info->param, info->conn_id);
	if (!agent) {
		LogError("create new login agent failed with login_id(%d) and conn_id(%d)", login_id, info->conn_id);
		return -1;
	}

	LogInfo("processed connect login server %d response", login_id);
	return 0;
}

// process selected gate server notify
int ConnLoginHandler::processSelectedServerNotify(JmyMsgInfo* info)
{
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) {
		LogError("login connection with id(%d) not found", info->conn_id);
		return -1;
	}

	MsgLS2GT_SelectedServerNotify notify;
	if (!notify.ParseFromArray(info->data, info->len)) {
		LogError("parse MsgLS2GT_SelectedServerNotify failed");
		return -1;
	}
	// generate session code to verify new client
	char* session_code = get_session_code(session_code_buff_, ENTER_GAME_SESSION_CODE_LENGTH);
	if (!CLIENT_MANAGER->newClientSession(notify.account().c_str(), session_code)) {
		LogError("create new account(%s) and session(%s) failed", notify.account().c_str(), session_code);
		return -1;
	}

	MsgGT2LS_SelectedServerResponse response;
	response.set_account(notify.account());
	response.set_session_code(std::string(session_code));
	if (!response.SerializeToArray(tmp_, sizeof(tmp_))) {
		LogError("serialize MsgGT2LS_SelectedServerResponse failed");
		return -1;
	}

	if (conn->send(MSGID_GT2LS_SELECTED_SERVER_RESPONSE, tmp_, response.ByteSize()) < 0) {
		LogError("send MsgGT2LS_SelectedServerResponse failed");
		return -1;
	}

	LogInfo("account %s select notify with session_code(%s)", notify.account().c_str(), session_code);
	return 0;
}

