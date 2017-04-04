#include "conn_config_msg_handler.h"
#include "../libjmy/jmy_datatype.h"
#include "../common/util.h"
#include "../../proto/src/server.pb.h"

int ConnConfigMsgHandler::processConnectResponse(JmyMsgInfo* info)
{
	MsgCS2LS_ConnectResponse response;
	if (!response.SerializeToArray(info->data, info->len)) {
		ServerLogError("serilize config_server to login_server connect response message failed");		
		return -1;
	}	
	return 0;
}
