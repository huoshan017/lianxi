#pragma once

#include <unordered_map>
#include <boost/asio.hpp>
#include "jmy_datatype.h"
#include "jmy_session_buffer.h"

using namespace boost::asio;

class JmyTcpConnector : public std::enable_shared_from_this<JmyTcpConnector>
{
public:
	JmyTcpConnector();
	JmyTcpConnector(io_service& service, const ip::tcp::endpoint& ep);
	~JmyTcpConnector();

	bool init(io_service& service);
	void close();
	void destroy();

	bool loadConfig(const JmyConnectorConfig& conf);
	void connect(const ip::tcp::endpoint& ep);
	int send(const char* data, unsigned int len);
	int run();

	std::shared_ptr<ip::tcp::socket> getSock() { return sock_; }

private:
	std::shared_ptr<ip::tcp::socket> sock_;
	ip::tcp::endpoint ep_;
	JmySessionBuffer recv_buff_;
	JmySessionBuffer send_buff_;
	JmyConnectorConfig conf_;
};

class JmyTcpMultiSameConnectors
{
public:
	JmyTcpMultiSameConnectors();
	JmyTcpMultiSameConnectors(io_service& service, const ip::tcp::endpoint& ep);
	~JmyTcpMultiSameConnectors();

	bool init(io_service& service);
	void close();
	void destroy();

	bool loadConfig(const JmyMultiSameConnectorsConfig& conf);
	bool start();
	int send(int connector_id, const char* data, unsigned int len);
	int run();

private:
	std::unordered_map<int, std::shared_ptr<ip::tcp::socket> > id2socks_;
	ip::tcp::endpoint ep_;
	int curr_id_;
};
