#include "login_handler.h"
#include "../libjmy/jmy_mem.h"
#include "../libjmy/jmy_const.h"
#include "../common/util.h"
#include "../../proto/src/server.pb.h"


char LoginHandler::tmp_[MAX_SEND_BUFFER_SIZE];
LoginAgentManager LoginHandler::login_mgr_;
LoginHandler::account2data_type LoginHandler::account2datas_;
char LoginHandler::session_code_buff_[ENTER_GAME_SESSION_CODE_LENGTH];

int LoginHandler::processSelectedServerNotify(JmyMsgInfo* info)
{
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) {
		ServerLogError("login connection with id(%d) not found", info->session_id);
		return -1;
	}

	MsgLS2GT_SelectedServerNotify notify;
	notify.ParseFromArray(info->data, info->len);
	AccountData* account_data = getAccountData(notify.account());
	if (account_data) {
		ServerLogError("account(%s) already exist", notify.account().c_str());
		return -1;
	}

	AccountData* data = jmy_mem_malloc<AccountData>();
	data->session_code = get_session_code(session_code_buff_, ENTER_GAME_SESSION_CODE_LENGTH);
	account2datas_.insert(std::make_pair(notify.account(), data));

	MsgGT2LS_SelectedServerResponse response;
	response.set_account(notify.account());
	response.set_session_code(session_code_buff_);
	response.SerializeToArray(tmp_, sizeof(tmp_));
	conn->send(MSGID_GT2LS_SELECTED_SERVER_RESPONSE, tmp_, response.ByteSize());
	return 0;
}

int LoginHandler::onConnect(JmyEventInfo* info)
{
	return 0;
}

int LoginHandler::onDisconnect(JmyEventInfo* info)
{
	return 0;
}

int LoginHandler::onTick(JmyEventInfo* info)
{
	return 0;
}

int LoginHandler::onTimer(JmyEventInfo* info)
{
	return 0;
}

LoginHandler::AccountData* LoginHandler::getAccountData(const std::string& account)
{
	account2data_type::iterator it = account2datas_.find(account);
	if (it == account2datas_.end())
		return nullptr;

	return it->second;
}

bool LoginHandler::removeAccountData(const std::string& account)
{
	AccountData* account_data = getAccountData(account);
	if (!account_data)
		return false;

	jmy_mem_free(account_data);
	return true;
}
