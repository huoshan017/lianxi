#pragma once

#include <unordered_map>
#include "my_datatype.h"

class MyDataHandler
{
public:
	MyDataHandler();
	~MyDataHandler();
	bool loadMsgHandle(const MyId2MsgHandler id2handlers[], int size);
	int processData(const char* data, unsigned int len, int session_id);
private:
	int processMsg(int msg_id, const char* data, unsigned int len, int session_id);
private:
	std::unordered_map<int, my_msg_handler> msg_handler_map_;
};
