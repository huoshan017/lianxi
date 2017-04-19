#pragma once

#include <string>
#include <list>
#include "../common/util.h"

enum UserEventType {
	USER_EVENT_NONE = 0,
	USER_EVENT_CONNECT_GAME_SERVER = 1,
	USER_EVENT_EXIT_GAME = 2,
};

struct UserEvent {
	int event_id;
	void* ptr_param;
	long l_param;
	std::string str_param;
	UserEvent() : event_id(0), ptr_param(nullptr), l_param(0) {
	}
	UserEvent(int eid, void* pparam, long lparam, const std::string& sparam) : event_id(eid), ptr_param(pparam), l_param(lparam), str_param(sparam) {
	}
};

class TestClient;
class UserEventList
{
public:
	UserEventList() {}
	~UserEventList() {}

	bool pushEvent(int event_id, void* ptr_param, long l_param, const std::string& str_param) {
		events_.emplace_back(event_id, ptr_param, l_param, str_param);
		return true;
	}

	bool popEvent(UserEvent& event) {
		if (events_.size() == 0)
			return false;
		event = std::move(events_.front());
		events_.pop_front();
		return true;
	}

private:
	std::list<UserEvent> events_;
};
