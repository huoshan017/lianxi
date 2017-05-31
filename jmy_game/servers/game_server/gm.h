#pragma once
#include <string>
#include <vector>
#include <unordered_map>

typedef int (*gm_command_func)(uint64_t role_id, const std::vector<std::string>&); 

struct GmCommand {
	const char* cmd;
	gm_command_func func;
};

class GmManager {
public:
	GmManager();
	~GmManager();
	bool load_cmds(const GmCommand cmds[], int cmd_num);
	bool parse_command(const std::string& command, std::vector<std::string>& results);
	bool execute_command(uint64_t role_id, const std::vector<std::string>& results);

private:
	std::unordered_map<std::string, const GmCommand*> cmd_map_;
};
