#include "game_handler.h"
#include "../libjmy/jmy_datatype.h"
#include "../libjmy/jmy_tcp_connection.h"
#include "../common/util.h"
#include "../../proto/src/common.pb.h"
#include "config_loader.h"

char GameHandler::tmp_[JMY_MAX_MSG_SIZE];
std::string GameHandler::enter_session_;
std::string GameHandler::reconn_session_;

int GameHandler::onConnect(JmyEventInfo* info)
{
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) {
		LogError("get connection failed in onConnect");
		return -1;
	}

	MsgC2S_EnterGameRequest request;
	request.set_account(CLIENT_CONFIG.account);
	request.set_session_code(enter_session_);
	if (!request.SerializeToArray(tmp_, sizeof(tmp_))) {
		LogError("serialize msg MsgC2S_EnterGameRequest failed");
		return -1;
	}
	if (conn->send(MSGID_C2S_ENTER_GAME_REQUEST, tmp_, request.ByteSize()) < 0) {
		LogError("send msg MsgC2S_EnterGameRequest failed");
		return -1;
	}
	LogInfo("game onConnect");
	return 0;
}

int GameHandler::onDisconnect(JmyEventInfo* info)
{
	(void)info;
	LogInfo("game onDisconnect");
	return 0;
}

int GameHandler::onTick(JmyEventInfo* info)
{
	(void)info;
	return 0;
}

int GameHandler::processEnterGame(JmyMsgInfo* info)
{
	MsgS2C_EnterGameResponse response;
	if (!response.ParseFromArray(info->data, info->len)) {
		LogError("parse msg MsgS2C_EnterGameResponse failed");
		return -1;
	}
	reconn_session_ = response.reconnect_session();
	LogInfo("processEnterGame");
	return info->len;
}

int GameHandler::processReconnect(JmyMsgInfo* info)
{
	return info->len;
}

int GameHandler::processDefault(JmyMsgInfo* info)
{
	return info->len;
}

void GameHandler::setEnterSession(const std::string& session)
{
	enter_session_ = std::move(session);
}
