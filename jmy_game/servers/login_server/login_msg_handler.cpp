#include "login_msg_handler.h"
#include "../libjmy/jmy.h"
#include "util.h"
#include "user.h"

char LoginMsgHandler::tmp_[MAX_SEND_BUFFER_SIZE];

void LoginMsgHandler::send_login_error(JmyTcpConnection* conn, MsgL2CLoginResponse& response, int error) {
	response.set_error_code(error);
	conn->send(MSGID_L2C_LOGIN_RESPONSE, tmp_, response.ByteSize());
}

int LoginMsgHandler::processLogin(JmyMsgInfo* info)
{
	MsgL2CLoginResponse response;
	MsgC2LLoginRequest request;
	if (!request.ParseFromArray(info->data, info->len)) {
		JmyTcpConnection* conn = get_connection(info);
		if (conn) {
			send_login_error(conn, response, 1);
		}
		ServerLogError("parse MsgC2LLoginRequest failed");
		return -1;
	}

	request.account();
	request.password();

	User* user = USER_MGR->newUser(info->session_id, (JmyTcpConnectionMgr*)info->param, request.account().c_str());
	if (!user) {
		JmyTcpConnection* conn = get_connection(info);
		if (conn) {
			send_login_error(conn, response, 2);
		}
		ServerLogError("create user by account(%s) failed", request.account().c_str());
		return -1;
	}

	response.set_error_code(0);
	response.SerializeToArray(tmp_, sizeof(tmp_));

	user->sendMsg(MSGID_L2C_LOGIN_RESPONSE, tmp_, response.ByteSize());
	ServerLogInfo("account(%d) login", request.account().c_str());

	return 0;
}

int LoginMsgHandler::processSelectServer(JmyMsgInfo* info)
{
	return 0;
}

int LoginMsgHandler::processEnterGame(JmyMsgInfo* info)
{
	return 0;
}
