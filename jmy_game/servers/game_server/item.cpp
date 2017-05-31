#include "item.h"
#include "../libjmy/jmy_mem.h"

ItemMgr::ItemMgr()
{
}

ItemMgr::~ItemMgr()
{
}

bool ItemMgr::add_item(uint32_t type_id, uint32_t item_num)
{
	std::unordered_map<uint32_t, Item*>::iterator it = items_.find(type_id);
	if (it == items_.end()) {
		Item* item = jmy_mem_malloc<Item>();
		item->type_id = type_id;
		item->item_num = item_num;
		items_.insert(std::make_pair(type_id, item));
	} else {
		if (!it->second)
			return false;
		it->second->item_num += item_num;
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
