#include "client_msg_handler.h"
#include "../libjmy/jmy.h"
#include "../../proto/src/common.pb.h"
#include "../common/util.h"

char ClientMsgHandler::tmp_[MAX_SEND_BUFFER_SIZE];

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
	MsgC2TEnterGameRequest request;
	if (!request.ParseFromArray(info->data, info->len)) {
		send_error(info, PROTO_ERROR_LOGIN_DATA_INVALID);
		return -1;
	}
	return 0;
}
