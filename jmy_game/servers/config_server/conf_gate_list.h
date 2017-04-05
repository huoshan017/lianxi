#pragma once

#include "../libjmy/jmy_singleton.hpp"
#include "../common/defines.h"
#include "../thirdparty/include/rapidjson/document.h"
#include "../thirdparty/include/rapidjson/stringbuffer.h"
#include "../thirdparty/include/rapidjson/writer.h"
#include "../../proto/src/server.pb.h"
#include <list>
#include <array>

enum { MAX_GATE_SERVER_COUNT = GATE_SERVER_MAX_ID-GATE_SERVER_MIN_ID };
class ConfGateList : public JmySingleton<ConfGateList>
{
public:
	ConfGateList();
	~ConfGateList();

	bool loadJson(const char* jsonpath);
	bool reload();
	void clear();
	void reset();

	size_t getSize() const { return gate_count_; }

	/*struct GateData {
		std::string name;
		int id;
		std::string ip;
		short port;
	};*/

	MsgGateConfData* get(int index);

private:
	rapidjson::Document doc_;
	std::string jsonpath_;
	std::array<MsgGateConfData*, MAX_GATE_SERVER_COUNT> gate_array_;
	size_t gate_count_;
};

#define CONF_GATE_LIST (ConfGateList::getInstance())
