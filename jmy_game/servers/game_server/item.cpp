#include "item.h"
#include "../libjmy/jmy_mem.h"
#include "../common/util.h"
#include "csv_list_manager.h"

ItemMgr::ItemMgr()
{
}

ItemMgr::~ItemMgr()
{
}

bool ItemMgr::add_item(uint32_t type_id, uint32_t item_num)
{
	item* item_def = CSV_MGR->get_item_table().get(type_id);
	if (!item_def) {
		LogError("get item(%d) define failed", type_id);
		return false;
	}

	std::unordered_map<uint32_t, Item*>::iterator it = items_.find(type_id);
	if (it == items_.end()) {
		Item* item_obj = jmy_mem_malloc<Item>();
		item_obj->type_id = type_id;
		if (item_def->max_count < (int)item_num)
			item_obj->item_num = item_def->max_count;
		else
			item_obj->item_num = item_num;
		items_.insert(std::make_pair(type_id, item_obj));
	} else {
		if (!it->second)
			return false;
		it->second->item_num += item_num;
		if ((int)it->second->item_num > item_def->max_count)
			it->second->item_num = item_def->max_count;
	}
	return true;
}

bool ItemMgr::rm_item(uint32_t type_id, uint32_t item_num)
{
	std::unordered_map<uint32_t, Item*>::iterator it = items_.find(type_id);
	if (it == items_.end())
		return false;
	if (!it->second)
		return false;
	if (it->second->item_num < item_num)
		return false;
	it->second->item_num -= item_num;
	return true;
}
