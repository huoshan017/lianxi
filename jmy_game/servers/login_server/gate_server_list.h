#pragma once

#include <list>
#include "../libjmy/jmy_singleton.hpp"
#include "../../proto/src/server.pb.h"

class GateServerList : public JmySingleton<GateServerList>
{
public:
	GateServerList() {}
	~GateServerList() { clear(); }

	void clear() {
		gate_conf_list_.clear();
	}
	void push(MsgGateConfData& data) {
		gate_conf_list_.push_back(std::move(data));
	}

	typedef std::list<MsgGateConfData>::iterator Iterator;
	Iterator begin() {
		return gate_conf_list_.begin();
	}
	Iterator end() {
		return gate_conf_list_.end();
	}

private:
	std::list<MsgGateConfData> gate_conf_list_;
};

#define GATE_SERVER_LIST (GateServerList::getInstance())
