#include "login_handler.h"
#include "../libjmy/jmy_datatype.h"
#include "../libjmy/jmy_tcp_connection.h"
#include "../common/util.h"
#include "../../proto/src/msgid.pb.h"
#include "../../proto/src/login.pb.h"
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
	
	LogInfo("account(%s) login onConnect", account.c_str());
	return 0;
}

int LoginHandler::onDisconnect(JmyEventInfo* info)
{
	std::string account;
	if (!CLIENT_MGR->getAccountByConnId(info->conn_id, account)) {
		LogError("cant get account by conn_id(%d)", info->conn_id);
		return -1;
	}
	LogInfo("account(%s) login onDisconnect", account.c_str());
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

#if 0
	MsgC2S_EchoRequest echo_req;
	for (i=0; i<100000; ++i) {
		echo_req.set_echo_str(std::to_string(i));
		if (!echo_req.SerializeToArray(tmp_, sizeof(tmp_))) {
			LogError("serialize MsgS2C_EchoResponse failed");
			return -1;
		}
		if (conn->send(MSGID_C2S_ECHO_REQUEST, tmp_, echo_req.ByteSize()) < 0) {
			LogError("send MsgS2C_EchoResponse failed");
			return -1;
		}
	}
#endif

#if 1
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
#endif

	std::string account;
	if (!CLIENT_MGR->getAccountByConnId(info->conn_id, account)) {
		LogError("cant get account by conn_id(%d)", info->conn_id);
		return -1;
	}

	static int login_count = 0;
	LogInfo("processLogin: get %d servers info, account(%s), count(%d)", s, account.c_str(), ++login_count);
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
		LogError("get account by conn_id(%d) failed", info->conn_id);
		return -1;
	}

	client->setEnterSession(response.session_code());
#if 1
	client->postConnectGameEvent(response.server_ip().c_str(), response.port());
#endif
	LogInfo("processSelectedServer: session_code(%s), gate_ip(%s), gate_port(%d)",
			response.session_code().c_str(), response.server_ip().c_str(), response.port());
	return info->len;
}

int LoginHandler::processEcho(JmyMsgInfo* info)
{
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) {
		LogError("get connection failed");
		return -1;
	}

	MsgS2C_EchoResponse response;
	if (!response.ParseFromArray(info->data, info->len)) {
		LogError("parse MsgS2C_EchoResponse failed");
		return -1;
	}

	TestClient* client = CLIENT_MGR->getClientByConnId(info->conn_id);
	if (!client) {
		LogError("get client by conn_id(%d) failed", info->conn_id);
		return -1;
	}

	static int echo_count = 0;
	LogInfo("get echo response str(%s) count(%d)", response.echo_str().c_str(), ++echo_count);
	return info->len;
}

int LoginHandler::processDefault(JmyMsgInfo* info)
{
	return info->len;
}
