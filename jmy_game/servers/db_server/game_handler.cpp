#include "game_handler.h"
#include "../common/util.h"
#include "../../proto/src/server.pb.h"
#include "game_server_manager.h"
#include "dbres_callback_funcs.h"
#include "global_data.h"
#include "db_server.h"
#include "../mysql/mysql_defines.h"
#include "db_tables_func.h"
#include "db_tables_struct.h"

char GameHandler::tmp_[JMY_MAX_MSG_SIZE];

int GameHandler::onConnect(JmyEventInfo* info)
{
	LogInfo("onconnection conn_id(%d)", info->conn_id);
	return 0;
}

int GameHandler::onDisconnect(JmyEventInfo* info)
{
	if (GAME_MGR->getByConnId(info->conn_id)) {
		GAME_MGR->removeByConnId(info->conn_id);
	}
	LogInfo("ondisconnect conn_id(%d)", info->conn_id);
	return 0;
}

int GameHandler::onTick(JmyEventInfo* info)
{
	(void)info;
	return 0;
}

int GameHandler::processConnectDBRequest(JmyMsgInfo* info)
{
	if (GAME_MGR->getByConnId(info->conn_id)) {
		LogError("already exist game agent by conn_id(%d)", info->conn_id);
		return -1;
	}
	MsgGS2DS_ConnectDBRequest request;
	if (!request.ParseFromArray(info->data, info->len)) {
		LogError("parse MsgGS2DS_ConnectDBRequest failed");
		return -1;
	}
	int game_id = request.game_id();
	GameAgent* agent = GAME_MGR->get(game_id);
	if (!agent) {
		agent = GAME_MGR->newAgent(game_id, (JmyTcpConnectionMgr*)info->param, info->conn_id);
		if (!agent) {
			LogError("create game agent with game_id(%d), conn_id(%d) failed", game_id, info->conn_id);
			return -1;
		}
	}

	MsgDS2GS_ConnectDBResponse response;
	if (!response.SerializeToArray(tmp_, sizeof(tmp_))) {
		LogError("serialize MsgDS2GS_ConnectDBResponse failed");
		return -1;
	}
	if (agent->sendMsg(MSGID_DS2GS_CONNECT_DB_RESPONSE, tmp_, response.ByteSize()) < 0) {
		LogError("send MsgDS2GS_ConnectDBResponse to game server %d failed", game_id);
		return -1;
	}

	LogInfo("game server %d connected", game_id);
	return info->len;
}

int GameHandler::processGetRole(JmyMsgInfo* info)
{
	MsgGS2DS_GetRoleRequest request;
	if (!request.ParseFromArray(info->data, info->len)) {
		LogError("parse MsgGS2DS_GetRoleRequest failed");
		return -1;
	}

	mysql_records_manager2<t_player, uint64_t, std::string>& player_mgr = TABLES_MGR.get_t_player_table();
	t_player* user = player_mgr.get_by_key2(request.account());
	if (!user) {
		user = player_mgr.get_new_by_key2(request.account());
		user->set_account(request.account());
		if (!db_select_t_player_fields_by_account(user->get_account(), DBResCBFuncs::getPlayerInfo, (void*)&user->get_account(), (long)info->conn_id)) {
			LogError("select account(%s) record failed", user->get_account().c_str());
			return -1;
		}
		LogInfo("to selecting record by account(%s)", user->get_account().c_str());
	} else {
		if (DBResCBFuncs::sendGetRoleResponse(user, info->conn_id) < 0) {
			LogError("send get account(%s) role response failed", request.account().c_str());
			return -1;
		}
		LogInfo("send get user(addr:0x%x, account:%s) response", user, request.account().c_str());
	}

	static int get_count = 0;
	LogInfo("process get role count: %d", ++get_count);

	return info->len;
}

static inline uint64_t gen_unique_role_id(int server_id) {
	uint64_t id = server_id;
	id = (id<<48) & 0xffff000000000000;
	id += (std::time(0)<<16) & 0xffff0000;
	static uint16_t counter = 0;
	id += counter;
	counter += 1;
	return id;
}

int GameHandler::processCreateRole(JmyMsgInfo* info)
{
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) return -1;

	MsgGS2DS_CreateRoleRequest request;
	if (!request.ParseFromArray(info->data, info->len)) {
		LogError("parse MsgGS2DS_CreateRoleRequest failed");
		return -1;
	}

	mysql_records_manager2<t_player, uint64_t, std::string>& player_mgr = TABLES_MGR.get_t_player_table();
	t_player* user = player_mgr.get_by_key2(request.account());
	if (!user) {
		user = player_mgr.get_new_by_key2(request.account());
	}

	user->set_account(request.account());
	int game_id = GAME_MGR->getIdByConnId(info->conn_id);
	uint64_t role_id = gen_unique_role_id(game_id);
	user->set_role_id(role_id);
	user->set_sex(request.sex());
	user->set_nick_name(request.nick_name());
	if (!player_mgr.insert_key_record(role_id, user)) {
		LogError("insert key(role_id:%llu) record failed", role_id);
		return -1;
	}
	t_player* user2 = player_mgr.get_by_key(role_id);
	if (!user2) {
		LogError("not found t_player by role_id(%llu)", role_id);
		return -1;
	}

	GLOBAL_DATA->setAccount2UserId(user->get_account(), role_id);
	if (!player_mgr.commit_insert_request(user, nullptr, nullptr, 0)) {
		LogError("commit insert request t_player(account:%s, role_id:%llu) failed", user->get_account().c_str(), role_id);
		return -1;
	}

	MsgDS2GS_CreateRoleResponse response;
	response.set_account(request.account());
	response.mutable_role_data()->set_nick_name(request.nick_name());
	response.mutable_role_data()->set_sex(request.sex());
	response.mutable_role_data()->set_race(request.race());
	response.mutable_role_data()->set_role_id(role_id);
	if (!response.SerializeToArray(tmp_, sizeof(tmp_))) {
		LogError("serialize MsgDS2GS_CreateRoleResponse failed");
		return -1;
	}

	if (conn->send(MSGID_DS2GS_CREATE_ROLE_RESPONSE, tmp_, response.ByteSize()) < 0) {
		LogError("send MsgDS2GS_CreateRoleResponse failed");
		return -1;
	}

	static int create_count = 0;
	LogInfo("process create role count: %d", ++create_count);

	return info->len;
}

int GameHandler::processSetRoleData(JmyMsgInfo* info)
{
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) return -1;
	MsgGS2DS_SetRoleDataRequest request;
	if (!request.ParseFromArray(info->data, info->len)) {
		LogError("parse MsgGS2DS_SetRoleDataRequest failed");
		return -1;
	}

	mysql_records_manager2<t_player, uint64_t, std::string>& player_mgr = TABLES_MGR.get_t_player_table();
	t_player* p = player_mgr.get_by_key(request.role_id());
	if (!p) {
		LogError("get t_player failed by role_id(%llu)", request.role_id());
		return -1;
	}
	const MsgBaseRoleData& role_data = request.role_data();
	p->set_sex(role_data.sex());
	p->set_level(role_data.level());
	player_mgr.commit_update_request_by_key(request.role_id());

	MsgDS2GS_SetRoleDataResponse response;
	response.mutable_role_data()->set_sex(role_data.sex());
	response.mutable_role_data()->set_level(role_data.level());
	if (!response.SerializeToArray(tmp_, sizeof(tmp_))) {
		LogError("serialize MsgDS2GS_SetRoleDataResponse failed");
		return -1;
	}
	if (conn->send(MSGID_DS2GS_SET_ROLE_DATA_RESPONSE, tmp_, response.ByteSize()) < 0) {
		LogError("send MsgDS2GS_SetRoleDataResponse failed");
		return -1;
	}
	LogInfo("set role(role_id: %llu) data", request.role_id());
	return info->len;
}

int GameHandler::processAddItem(JmyMsgInfo* info)
{
	JmyTcpConnection* conn = get_connection(info);
	if (!conn) return -1;
	MsgGS2DS_AddItemRequest request;
	if (!request.ParseFromArray(info->data, info->len)) {
		LogError("parse MsgGS2DS_AddItemRequest failed");
		return -1;
	}
	mysql_records_manager2<t_player, uint64_t, std::string>& player_mgr = TABLES_MGR.get_t_player_table();
	t_player* p = player_mgr.get_by_key(request.role_id());
	if (!p) {
		LogError("get t_player failed by role_id(%llu)", request.role_id());
		return -1;
	}

	uint32_t type_id = request.type_id();
	uint32_t item_num = request.item_num();
	auto item_list = p->get_mutable_items().mutable_item_list();
	auto it = item_list->find(type_id);
	if (it == item_list->end()) {
		DBItemData* item_data = jmy_mem_malloc<DBItemData>();
		item_data->set_type_id(type_id);
		item_data->set_num(item_num);
		item_list->insert(::google::protobuf::Map< ::google::protobuf::int32, ::DBItemData >::value_type(type_id, *item_data));
	} else {
		it->second.set_num(it->second.num()+item_num);
	}
	p->commit_items_changed();
	player_mgr.commit_update_request(p);

	MsgDS2GS_AddItemResponse response;
	response.set_role_id(request.role_id());
	response.set_type_id(request.type_id());
	response.set_item_num(request.item_num());
	if (!response.SerializeToArray(tmp_, sizeof(tmp_))) {
		LogError("serialize MsgDS2GS_AddItemResponse failed");
		return -1;
	}
	if (conn->send(MSGID_DS2GS_ADD_ITEM_RESPONSE, tmp_, response.ByteSize()) < 0) {
		LogError("send MsgDS2GS_AddItemResponse failed");
		return -1;
	}
	LogInfo("add item(type_id:%u, item_num:%d)", type_id, item_num);
	return info->len;
}

int GameHandler::processRmItem(JmyMsgInfo* info)
{
	return info->len;
}

int GameHandler::processDefault(JmyMsgInfo* info)
{
	switch (info->msg_id) {
	case MSGID_GS2DS_SET_ROLE_DATA_REQUEST:
		return processSetRoleData(info);
	case MSGID_GS2DS_ADD_ITEM_REQUEST:
		return processAddItem(info);
	default:
		break;
	}
	return info->len;
}
