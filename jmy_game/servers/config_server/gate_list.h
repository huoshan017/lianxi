#pragma once

#include "../libjmy/jmy_singleton.hpp"
#include "../libjmy/thirdparty/include/rapidjson/document.h"
#include "../libjmy/thirdparty/include/rapidjson/stringbuffer.h"
#include "../libjmy/thirdparty/include/rapidjson/writer.h"
#include <array>
#include <string>

enum { MAX_GATE_SERVER_COUNT = 100 };
class ConfGateList : public JmySingleton<ConfGateList>
{
public:
	ConfGateList();
	~ConfGateList();

	bool loadJson(const char* jsonpath);
	bool reload();
	void close();
	void clear();

	size_t getGateCount() const { return gate_count_; }

	struct GateData {
		std::string name;
		int id;
		std::string ip;
		short port;
	};

	GateData* getGate(int index);

private:
	rapidjson::Document doc_;
	std::string jsonpath_;
	std::array<GateData*, MAX_GATE_SERVER_COUNT> gate_array_;
	size_t gate_count_;
};

#define CONF_GATE_LIST (ConfGateList::getInstance())
