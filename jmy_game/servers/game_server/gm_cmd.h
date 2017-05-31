#pragma once

#include <vector>
#include <string>
#include "gm.h"

int gm_add_item(uint64_t role_id, const std::vector<std::string>& params);
int gm_rm_item(uint64_t role_id, const std::vector<std::string>& params);
int gm_levelup(uint64_t role_id, const std::vector<std::string>& params);
int gm_setlevel(uint64_t role_id, const std::vector<std::string>& params);
int gm_addexp(uint64_t role_id, const std::vector<std::string>& params);

static const GmCommand gm_cmds[] = {
	{ "additem", gm_add_item },
	{ "rmitem", gm_rm_item },
	{ "levelup", gm_levelup },
	{ "setlevel", gm_setlevel },
	{ "addexp", gm_addexp }
};
