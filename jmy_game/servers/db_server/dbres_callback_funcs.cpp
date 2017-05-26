#include "dbres_callback_funcs.h"
#include "../common/util.h"
#include "../../proto/src/server.pb.h"
#include "global_data.h"
#include "db_server.h"
#include "game_server_manager.h"
#include "db_tables_func.h"
#include <string>

char DBResCBFuncs::tmp_[4096*128];

int DBResCBFuncs::getAllAccounts(MysqlConnector::Result& res, void* param, long param_l)
{
	(void)param;
	(void)param_l;
	if (res.res_err != 0) {
		LogError("get all accounts failed, err(%d)", res.res_err);
		return -1;
	}

	int num = res.num_rows();
	if (num == 0 || res.is_empty()) {
		LogInfo("get all accounts result is empty");
		return 0;
	}

	char** datas = nullptr;
	while (true) {
		datas = res.fetch();
		if (!datas) break;
		if (res.num_fields() < 1) {
			LogError("error: result field num less 1");
			return -1;
		}
		const char* account = datas[0];
		GLOBAL_DATA->insertDBAccount(std::string(account));
	}

	LogInfo("accounts num is %d", num);

	return 0;
}

int DBResCBFuncs::getPlayerInfo(MysqlConnector::Result& res, void* param, long param_l)
{
	const std::string& account = *(std::string*)param;
	mysql_records_manager2<t_player, uint64_t, std::string>& player_mgr = TABLES_MGR.get_t_player_table();
	t_player* user = player_mgr.get_by_key2(account);
	if (!user) {
		LogError("cant get free t_player by account(%s)", account.c_str());
		return -1;
	}

	int r = db_get_result_of_select_t_player_by_account(res, user);
	if (r < 0) {
		LogError("get result of select player info failed");
		return -1;
	}

	if (sendGetRoleResponse(user, (int)param_l) < 0) {
		LogError("send get role response failed");
		return -1;
	}

	LogInfo("getPlayerInfo");
	return 0;
}

int DBResCBFuncs::sendGetRoleResponse(t_player* user, int conn_id)
{
	GameAgent* agent = GAME_MGR->getByConnId(conn_id);
	if (!agent) {
		LogError("not found game agent with conn_id(%d)", conn_id);
		return -1;
	}

	MsgDS2GS_GetRoleResponse response;
	MsgBaseRoleData* d = response.mutable_role_data();
	d->set_nick_name(user->get_nick_name());
	d->set_sex(user->get_sex());
	d->set_level(user->get_level());
	d->set_role_id(user->get_role_id());
	response.set_account(user->get_account());

	if (!response.SerializeToArray(tmp_, sizeof(tmp_))) {
		LogError("serialize MsgDS2GS_RequireUserDataResponse failed");
		return -1;
	}

	if (agent->sendMsg(MSGID_DS2GS_GET_ROLE_RESPONSE, tmp_, response.ByteSize()) < 0) {
		LogError("send MsgDS2GS_RequireUserDataResponse failed");
		return -1;
	}

	LogInfo("send get role response: role_id(%llu)", user->get_role_id());
	return 0;
}
