#pragma once

#include <stdint.h>
#include <unordered_map>

struct Item
{
	uint32_t type_id;
	uint32_t item_num;
	uint64_t unique_id;
};

class ItemMgr
{
public:
	ItemMgr();
	~ItemMgr();
	bool add_item(uint32_t type_id, uint32_t item_num);
	bool rm_item(uint32_t type_id, uint32_t item_num);

private:
	std::unordered_map<uint32_t, Item*> items_;
};
