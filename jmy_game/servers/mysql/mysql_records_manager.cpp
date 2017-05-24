#include "mysql_records_manager.h"
#include <iostream>

struct player {
	int id;
	std::string name;
	int sex;
	int race;
	bool insert_request(mysql_cmd_callback_func, void*, long) {
		return true;
	}
	bool delete_request() {
		return true;
	}
	bool update_request() {
		return true;
	}
};

int test() {
	mysql_records_subkey_manager<player, int, int> player_subkey_manager;
	mysql_record_list<player, int>* player_list = player_subkey_manager.get_new(1);
	player* p = player_list->get_new(1);
	p->name = "player1";
	p->sex = 0;
	p->race = 0;
	player_subkey_manager.remove_record(p);
	mysql_record_list<player, int>* player_list2 = player_subkey_manager.get_new(2);
	player* p2 = player_list2->get_new(1);
	p2->name = "player2";
	p2->sex = 1;
	p2->race = 1;
	player* pp = player_subkey_manager.get_record(2, 1);
	if (p2 != pp) {
		std::cout << "p2 != pp" << std::endl;
	}
	player_subkey_manager.update();

	// mysql_records_manager2
	mysql_records_manager2<player, int, std::string> player_manager2;
	player* p3 = player_manager2.get_new_by_key2("player3");
	p3->id = 3;
	player_manager2.make_pair(3, "player3");
	player* p33 = player_manager2.get_by_key(3);
	if (p3 != p33) {
		std::cout << "p3 != p33" << std::endl;
	}
	player_manager2.update();
	return 0;
}
