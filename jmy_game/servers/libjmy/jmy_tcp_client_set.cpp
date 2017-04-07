#include "jmy_tcp_client_set.h"
#include "jmy_tcp_client.h"

JmyTcpClientSet::JmyTcpClientSet()
{
}

JmyTcpClientSet::~JmyTcpClientSet()
{
	clear();
}

void JmyTcpClientSet::clear()
{
	client2config_map_.clear();
}

bool JmyTcpClientSet::addClient(JmyTcpClient* client, const JmyClientConfig* config)
{
	client2config_type::iterator it = client2config_map_.find(client);
	if (it != client2config_map_.end())
		return false;
	client2config_map_.insert(std::make_pair(client, config));
	return true;
}

bool JmyTcpClientSet::removeClient(JmyTcpClient* client)
{
	client2config_type::iterator it = client2config_map_.find(client);
	if (it == client2config_map_.end())
		return false;
	client2config_map_.erase(client);
	return true;
}

void JmyTcpClientSet::run()
{
	client2config_type::iterator it = client2config_map_.begin();
	for (; it!=client2config_map_.end(); ++it) {
		JmyTcpClient* c = it->first;
		const JmyClientConfig* cf = it->second; 
		if (!c || !cf) continue;
		if (cf->is_reconnect) {
			if (c->isDisconnected()) {
				c->reset();
			} else if (c->isNotConnect()) {
				c->reconnect(*cf);
			} else if (c->isConnecting()) {
			}
		}
		c->run();
	}
}
