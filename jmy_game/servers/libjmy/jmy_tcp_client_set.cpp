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
	client2data_map_.clear();
}

bool JmyTcpClientSet::addClient(JmyTcpClient* client, const JmyClientConfig* config)
{
	client2data_type::iterator it = client2data_map_.find(client);
	if (it != client2data_map_.end())
		return false;

	ClientData cd;
	cd.conf = const_cast<JmyClientConfig*>(config);
	client2data_map_.insert(std::make_pair(client, cd));
	return true;
}

bool JmyTcpClientSet::removeClient(JmyTcpClient* client)
{
	client2data_type::iterator it = client2data_map_.find(client);
	if (it == client2data_map_.end())
		return false;
	client2data_map_.erase(client);
	return true;
}

void JmyTcpClientSet::run()
{
	client2data_type::iterator it = client2data_map_.begin();
	for (; it!=client2data_map_.end(); ++it) {
		JmyTcpClient* c = it->first;
		const JmyClientConfig* cf = it->second.conf; 
		if (!c || !cf) continue;
		if (cf->is_reconnect) {
			if (c->isDisconnected()) {
				c->reset();
				it->second.last_point = std::chrono::system_clock::now();
			} else if (c->isNotConnect()) {
				std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
				if (std::chrono::duration_cast<std::chrono::seconds>(now - it->second.last_point).count() >= cf->reconnect_interval) {
					c->reconnect(*cf);
					it->second.last_point = now;
				}
			} else if (c->isConnecting()) {
			}
		}
		c->run();
	}
}
