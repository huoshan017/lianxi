#include "client_msg_handler.h"
#include "../libjmy/jmy.h"
#include "../common/util.h"
#include "../../proto/src/common.pb.h"

char ClientMsgHandler::tmp_[MAX_SEND_BUFFER_SIZE];
ClientAgentManager ClientMsgHandler::client_mgr_;

void ClientMsgHandler::send_error(JmyMsgInfo* info, ProtoErrorType error)
{
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) return;
	MsgError response;
	response.set_error_code(error);
	response.SerializeToArray(tmp_, sizeof(tmp_));
	conn->send(MSGID_ERROR, tmp_, response.ByteSize());
}

int ClientMsgHandler::processEnterGame(JmyMsgInfo* info)
{
	MsgCL2GT_EnterGameRequest request;
	if (!request.ParseFromArray(info->data, info->len)) {
		send_error(info, PROTO_ERROR_LOGIN_DATA_INVALID);
		return -1;
	}

	if (!CLIENT_MGR.getAgent(request.account())) {
		send_error(info, PROTO_ERROR_ENTER_GAME_REPEATED);
		ServerLogError("account(%s) already entered game", request.account().c_str());
		return -1;
	}

	ClientAgent* agent = CLIENT_MGR.newAgent(request.account(), (JmyTcpConnectionMgr*)info->param, info->session_id);
	if (!agent) {
		send_error(info, PROTO_ERROR_ENTER_GAME_FAILED);
		ServerLogError("create new client agent with id(%d) account(%s) failed", info->session_id, request.account().c_str());
		return -1;
	}



	return 0;
}
