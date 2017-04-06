#include "jmy_tcp_client_set.h"
#include "jmy_tcp_client.h"

JmyTcpClientSet::JmyTcpClientSet()
{
}

JmyTcpClientSet::~JmyTcpClientSet()
{
	clear();
}

bool JmyTcpClientSet::init(const JmyClientConfig& config)
{
	conf_ = config;
	return true;
}

void JmyTcpClientSet::clear()
{
	clients_.clear();
}

bool JmyTcpClientSet::addClient(JmyTcpClient* client)
{
	std::set<JmyTcpClient*>::iterator it = clients_.find(client);
	if (it != clients_.end())
		return false;
	clients_.insert(client);
	return true;
}

bool JmyTcpClientSet::removeClient(JmyTcpClient* client)
{
	std::set<JmyTcpClient*>::iterator it = clients_.find(client);
	if (it == clients_.end())
		return false;
	clients_.erase(client);
	return true;
}

void JmyTcpClientSet::run()
{
	std::set<JmyTcpClient*>::iterator it = clients_.begin();
	for (; it!=clients_.end(); ++it) {
		JmyTcpClient* c = *it;
		if (!c) continue;
		if (conf_.is_reconnect) {
			if (c->isDisconnected()) {
				c->reset();
			} else if (c->isNotConnect()) {
				c->reconnect(conf_);
			} else if (c->isConnecting()) {
			}
		}
		c->run();
	}
}
