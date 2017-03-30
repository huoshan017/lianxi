#include "login_msg_handler.h"
#include "../libjmy/jmy_mem.h"
#include "../libjmy/jmy_const.h"
#include "../common/util.h"
#include "../../proto/src/server.pb.h"


char LoginMsgHandler::tmp_[MAX_SEND_BUFFER_SIZE];
LoginAgentManager LoginMsgHandler::login_mgr_;
LoginMsgHandler::account2data_type LoginMsgHandler::account2datas_;

int LoginMsgHandler::processSelectedServerNotify(JmyMsgInfo* info)
{
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) {
		ServerLogError("login connection with id(%d) not found", info->session_id);
		return -1;
	}

	MsgL2TSelectedServerNotify notify;
	notify.ParseFromArray(info->data, info->len);
	AccountData* account_data = getAccountData(notify.account());
	if (account_data) {
		ServerLogError("account(%s) already exist", notify.account().c_str());
		return -1;
	}

	AccountData* data = jmy_mem_malloc<AccountData>();
	data->session_code = get_session_code(session_code_buff_, ENTER_GAME_SESSION_CODE_LENGTH);
	account2datas_.insert(std::make_pair(notify.account(), data));

	MsgT2LSelectedServerResponse response;
	response.set_account(notify.account());
	response.set_session_code(session_code_buff_);
	response.SerializeToArray(tmp_, sizeof(tmp_));
	conn->send(MSGID_T2L_SELECTED_SERVER_RESPONSE, tmp_, response.ByteSize());
	return 0;
}

LoginMsgHandler::AccountData* LoginMsgHandler::getAccountData(const std::string& account)
{
	account2data_type::iterator it = account2datas_.find(account);
	if (it == account2datas_.end())
		return nullptr;

	return it->second;
}

bool LoginMsgHandler::removeAccountData(const std::string& account)
{
	AccountData* account_data = getAccountData(account);
	if (!account_data)
		return false;

	jmy_mem_free(account_data);
	return true;
}
