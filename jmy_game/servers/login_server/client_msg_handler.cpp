#include "client_msg_handler.h"
#include "../libjmy/jmy.h"
#include "../common/util.h"
#include "user.h"
#include <random>

static const int SESSION_CODE_LENGTH = 16;

char ClientMsgHandler::tmp_[MAX_SEND_BUFFER_SIZE];

void ClientMsgHandler::send_error(JmyMsgInfo* info, ProtoErrorType error) {
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) return;
	MsgError response;
	response.set_error_code(error);
	conn->send(MSGID_L2C_LOGIN_RESPONSE, tmp_, response.ByteSize());
}

int ClientMsgHandler::processLogin(JmyMsgInfo* info)
{
	MsgC2LLoginRequest request;
	if (!request.ParseFromArray(info->data, info->len)) {
		send_error(info, PROTO_ERROR_LOGIN_DATA_INVALID);
		ServerLogError("parse MsgC2LLoginRequest failed");
		return -1;
	}

	request.account();
	request.password();

	User* user = USER_MGR->newUser(info->session_id, (JmyTcpConnectionMgr*)info->param, request.account().c_str());
	if (!user) {
		send_error(info, PROTO_ERROR_LOGIN_REPEATED);
		ServerLogError("create user by account(%s) failed", request.account().c_str());
		return -1;
	}

	MsgL2CLoginResponse response;
	response.SerializeToArray(tmp_, sizeof(tmp_));

	user->setState(USER_STATE_VERIFIED);
	user->sendMsg(MSGID_L2C_LOGIN_RESPONSE, tmp_, response.ByteSize());
	ServerLogInfo("account(%d) login", request.account().c_str());

	return 0;
}

int ClientMsgHandler::processSelectServer(JmyMsgInfo* info)
{
	MsgC2LSelectServerRequest request;
	if (!request.ParseFromArray(info->data, info->len)) {
		send_error(info, PROTO_ERROR_LOGIN_DATA_INVALID);
		ServerLogError("parse MsgC2LSelectServerRequest failed");
		return -1;
	}

	User* user = USER_MGR->getUserById(info->session_id);
	if (!user) {
		send_error(info, PROTO_ERROR_LOGIN_ACCOUNT_OR_PASSWORD_INVALID);
		ServerLogError("cant find user by id(%d)", info->session_id);
		return -1;
	}

	MsgL2CSelectServerResponse response;
	static char cs[] = "abcdefghijklmnopqrstuvwxyz0123456789~!@#$%^&*()_+`-={}[]:<>?,./";
	// generate session string
	std::default_random_engine gen;
	std::uniform_int_distribution<> dis(0, sizeof(cs));
	char* session_code = (char*)jmy_mem_malloc(SESSION_CODE_LENGTH);
	for (int i=0; i<SESSION_CODE_LENGTH; ++i) {
		session_code[i] = dis(gen);
	}
	response.set_session_code(session_code);
	user->sendMsg(MSGID_L2C_SELECT_SERVER_RESPONSE, tmp_, response.ByteSize());
	ServerLogInfo("user(%s) select server", user->getAccount());

	return 0;
}
