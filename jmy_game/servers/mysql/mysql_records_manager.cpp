#include "mysql_records_manager.h"
#include <iostream>

struct player {
	int id;
	std::string name;
	int sex;
	int race;
	bool insert_request(/*mysql_cmd_callback_func, void*, long*/) {
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
	mysql_records_manager_map<player, int, int> player_subkey_manager;
	mysql_records_manager<player, int>* player_list = player_subkey_manager.get_new(1);
	player* p = player_list->get_new(1);
	p->name = "player1";
	p->sex = 0;
	p->race = 0;
	mysql_records_manager<player, int>* player_list2 = player_subkey_manager.get_new(2);
	player* p2 = player_list2->get_new(1);
	p2->name = "player2";
	p2->sex = 1;
	p2->race = 1;
	player_subkey_manager.update();

	// mysql_records_manager2
	mysql_records_manager2<player, int, std::string> player_manager2;
	player* p3 = player_manager2.get_new_by_key2("player3");
	p3->id = 3;
	player_manager2.insert_key_record(3, p3);
	player* p33 = player_manager2.get_by_key(3);
	if (p3 != p33) {
		std::cout << "p3 != p33" << std::endl;
	}
	player_manager2.update();
	return 0;
}

#include <boost/coroutine2/all.hpp>

int test_coroutine2() {
	typedef boost::coroutines2::coroutine<int> coro_t;
	coro_t::pull_type source(
		[&](coro_t::push_type& sink) {
			int first=1,second=1;
			sink(first);
			sink(second);
			for(int i=0;i<8;++i){
				int third=first+second;
				first=second;
				second=third;
				sink(third);
			}
		}
	);

	for(auto i:source)
		std::cout << i <<  " ";

	return 0;
}
