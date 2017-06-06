#include "login_handler.h"
#include "../libjmy/jmy_datatype.h"
#include "../libjmy/jmy_tcp_connection.h"
#include "../common/util.h"
#include "../../proto/src/common.pb.h"
#include "config_loader.h"
#include "test_client.h"
#include "game_handler.h"

char LoginHandler::tmp_[JMY_MAX_MSG_SIZE];

int LoginHandler::onConnect(JmyEventInfo* info)
{
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) {
		LogError("get connection failed in onConnect");
		return -1;
	}

	MsgC2S_LoginRequest request;
	std::string account;
	if (!CLIENT_MGR->getAccountByConnId(info->conn_id, account)) {
		LogError("cant get account by conn_id(%d)", info->conn_id);
		return -1;
	}

	request.set_account(account);
	if (!request.SerializeToArray(tmp_, sizeof(tmp_))) {
		LogError("serialize msg MsgC2S_LoginRequest failed");
		return -1;
	}

	if (conn->send(MSGID_C2S_LOGIN_REQUEST, tmp_, request.ByteSize()) < 0) {
		LogError("send msg MsgC2S_LoginRequest failed");
		return -1;
	}
	
	LogInfo("login onConnect");
	return 0;
}

int LoginHandler::onDisconnect(JmyEventInfo* info)
{
	(void)info;
	LogInfo("login onDisconnect");
	return 0;
}

int LoginHandler::onTick(JmyEventInfo* info)
{
	(void)info;
	return 0;
}

int LoginHandler::processLogin(JmyMsgInfo* info)
{
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) {
		LogError("get connection failed in processLogin");
		return -1;
	}
	
	MsgS2C_LoginResponse response;
	if (!response.ParseFromArray(info->data, info->len)) {
		LogError("parse MsgS2C_LoginResponse failed");
		return -1;
	}
	int s = response.servers_size();
	int i = 0;
	for (; i<s; ++i) {
		const MsgServerInfo& si = response.servers(i);
		LogInfo("server %d: name(%s), id(%d), is_maintenance(%d), is_busy(%d)",
				i+1, si.name().c_str(), si.id(), si.is_maintenance(), si.is_busy());
	}

	if (s > 0) {
		// select server
		MsgC2S_SelectServerRequest request;
		request.set_sel_id(response.servers(0).id());
		if (!request.SerializeToArray(tmp_, sizeof(tmp_))) {
			LogError("serialize msg MsgC2S_SelectServerRequest failed");
			return -1;
		}
		if (conn->send(MSGID_C2S_SELECT_SERVER_REQUEST, tmp_, request.ByteSize()) < 0) {
			LogError("send msg MsgC2S_SelectServerRequest failed");
			return -1;
		}
	}

	LogInfo("processLogin: get %d servers info", s);
	return info->len;
}

int LoginHandler::processSelectedServer(JmyMsgInfo* info)
{
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) {
		LogError("get connection failed in processSelectedServer");
		return -1;
	}

	MsgS2C_SelectServerResponse response;
	if (!response.ParseFromArray(info->data, info->len)) {
		LogError("parse msg MsgS2C_SelectServerResponse failed");
		return -1;
	}

	TestClient* client = CLIENT_MGR->getClientByConnId(info->conn_id);
	if (!client) {
		LogError("get account by conn_id(%d)", info->conn_id);
		return -1;
	}

	client->setEnterSession(response.session_code());
	client->postConnectGameEvent(response.server_ip().c_str(), response.port());
	LogInfo("processSelectedServer: session_code(%s), gate_ip(%s), gate_port(%d)",
			response.session_code().c_str(), response.server_ip().c_str(), response.port());
	return info->len;
}

int LoginHandler::processDefault(JmyMsgInfo* info)
{
	return info->len;
}
