#include "gm_cmd.h"
#include "global_data.h"
#include "player.h"
#include "item.h"
#include "../common/util.h"

int gm_add_item(uint64_t role_id, const std::vector<std::string>& params)
{
	uint32_t type_id = std::strtoul(params[2].c_str(), NULL, 10);
	uint32_t item_num = std::strtoul(params[3].c_str(), NULL, 10);
	Player* p = PLAYER_MGR->getByRoleId(role_id);
	if (!p) return -1;
	if (!p->items.add_item(type_id, item_num))
		return -1;
	LogInfo("gm command: add_item(type_id:%u, num:%d)", type_id, item_num);
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
	LogInfo("gm command: rm_item(type_id:%u, num:%d)", type_id, item_num);
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
