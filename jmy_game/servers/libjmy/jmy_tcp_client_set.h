#pragma once

#include "jmy_datatype.h"
#include <unordered_map>
#include <chrono>

class JmyTcpClient;
class JmyTcpClientSet
{
public:
	JmyTcpClientSet();
	~JmyTcpClientSet();

	bool init(const JmyClientConfig& config);
	void clear();

	bool addClient(JmyTcpClient* client, const JmyClientConfig* config);
	bool removeClient(JmyTcpClient* client);
	void run();

private:
	struct ClientData {
		JmyClientConfig* conf;
		std::chrono::system_clock::time_point last_point;
		ClientData() : conf(nullptr) {}
		ClientData(JmyClientConfig* c) : conf(c) {}
	};
	typedef std::unordered_map<JmyTcpClient*, ClientData> client2data_type;
	client2data_type client2data_map_;
};
