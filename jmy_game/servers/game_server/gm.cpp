#include "gm.h"
#include <boost/format.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#include "../common/util.h"

GmManager::GmManager()
{
}

GmManager::~GmManager()
{
}

bool GmManager::load_cmds(const GmCommand cmds[], int cmd_num)
{
	for (int i=0; i<cmd_num; ++i) {
		cmd_map_.insert(std::make_pair(cmds[i].cmd, &cmds[i]));
	}
	return true;
}

bool GmManager::parse_command(const std::string& command, std::vector<std::string>& results)
{
	boost::split(results, command, boost::is_any_of(" "));
	if (results.size() <= 1) {
		LogError("gm command not found params");
		return false;
	}
	if (results[0] != "/gm") {
		LogError("gm command format is invalid");
		return false;
	}
	return true;
}

bool GmManager::execute_command(uint64_t role_id, const std::vector<std::string>& results)
{
	if (results[0] != "/gm") {
		LogError("gm command format is invalid");
		return false;
	}
	std::unordered_map<std::string, const GmCommand*>::iterator it = cmd_map_.find(results[1]);
	if (it == cmd_map_.end() || !it->second) {
		LogError("not found gm command: %s", results[1].c_str());
		return false;
	}
	if (it->second->func(role_id, results) < 0) {
		LogError("gm command %s execute failed", it->second->cmd);
		return false;
	} 
	return true;
}
