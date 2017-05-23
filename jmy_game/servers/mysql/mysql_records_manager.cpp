#include "mysql_records_manager.h"

struct player {
	int id;
	std::string name;
	int sex;
	int race;
};

int test() {
	mysql_records_subkey_manager<player, int, int> player_subkey_manager;
	mysql_record_list<player, int>* player_list = player_subkey_manager.get_new(1);
	player* p = player_list->get_new(1);
	player_subkey_manager.remove_record(p);
	return 0;
}
