#include "gm_cmd.h"
#include "global_data.h"
#include "player.h"
#include "item.h"
#include "../common/util.h"
#include "../../proto/src/server.pb.h"

int gm_add_item(uint64_t role_id, const std::vector<std::string>& params)
{
	uint32_t type_id = std::strtoul(params[2].c_str(), NULL, 10);
	uint32_t item_num = std::strtoul(params[3].c_str(), NULL, 10);
	Player* p = PLAYER_MGR->getByRoleId(role_id);
	if (!p) return -1;
	if (!p->items.add_item(type_id, item_num))
		return -1;

	MsgGS2DS_AddItemRequest request;
	request.set_role_id(role_id);
	request.set_type_id(type_id);
	request.set_item_num(item_num);
	char tmp[128];
	if (!request.SerializeToArray(tmp, sizeof(tmp))) {
		LogError("serialize MsgGS2DS_AddItemRequest failed");
		return -1;
	}

	if (SEND_DB_MSG(MSGID_GS2DS_ADD_ITEM_REQUEST, tmp, request.ByteSize()) < 0) {
		LogError("send MsgGS2DS_AddItemRequest failed");
		return -1;
	}

	LogInfo("gm command: role(%llu) add_item(type_id:%u, num:%d)", role_id, type_id, item_num);
	return 0;
}

int gm_rm_item(uint64_t role_id, const std::vector<std::string>& params)
{
	uint32_t type_id = std::strtoul(params[2].c_str(), NULL, 10);
	uint32_t item_num = std::strtoul(params[3].c_str(), NULL, 10);
	Player* p = PLAYER_MGR->getByRoleId(role_id);
	if (!p) return -1;
	if (!p->items.rm_item(type_id, item_num))
		return -1;

	MsgGS2DS_RmItemRequest request;
	request.set_role_id(role_id);
	request.set_type_id(type_id);
	request.set_item_num(item_num);
	char tmp[128];
	if (!request.SerializeToArray(tmp, sizeof(tmp))) {
		LogError("serialize MsgGS2DS_RmItemRequest failed");
		return -1;
	}

	if (SEND_DB_MSG(MSGID_GS2DS_RM_ITEM_REQUEST, tmp, request.ByteSize()) < 0) {
		LogError("send MsgGS2DS_RmItemRequest failed");
		return -1;
	}

	LogInfo("gm command: role(%llu) rm_item(type_id:%u, num:%d)", role_id, type_id, item_num);
	return 0;
}

int gm_levelup(uint64_t role_id, const std::vector<std::string>& params)
{
	return 0;
}

int gm_setlevel(uint64_t role_id, const std::vector<std::string>& params)
{
	return 0;
}

int gm_addexp(uint64_t role_id, const std::vector<std::string>& params)
{
	return 0;
}
