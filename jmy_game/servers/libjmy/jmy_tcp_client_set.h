#pragma once

#include "jmy_datatype.h"
#include <set>

class JmyTcpClient;
class JmyTcpClientSet
{
public:
	JmyTcpClientSet();
	~JmyTcpClientSet();

	bool init(const JmyClientConfig& config);
	void clear();

	bool addClient(JmyTcpClient* client);
	bool removeClient(JmyTcpClient* client);
	void run();

private:
	std::set<JmyTcpClient*> clients_;
	JmyClientConfig conf_;
};
