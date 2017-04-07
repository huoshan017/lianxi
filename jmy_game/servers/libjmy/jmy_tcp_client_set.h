#pragma once

#include "jmy_datatype.h"
#include <unordered_map>

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
	typedef  std::unordered_map<JmyTcpClient*, const JmyClientConfig*> client2config_type;
	client2config_type client2config_map_;
};
