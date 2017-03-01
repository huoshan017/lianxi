#pragma once

#include <unordered_map>
#include <boost/asio.hpp>
#include "jmy_datatype.h"
#include "jmy_session_buffer.h"
#include "jmy_data_handler.h"

using namespace boost::asio;

enum JmyConnectorState {
	CONNECTOR_STATE_NOT_CONNECT = 0,
	CONNECTOR_STATE_CONNECTING = 1,
	CONNECTOR_STATE_CONNETED = 2,
	CONNECTOR_STATE_DISCONNECTING = 3,
	CONNECTOR_STATE_DISCONNECT = 4,
};

class JmyTcpConnector : public std::enable_shared_from_this<JmyTcpConnector>
{
public:
	JmyTcpConnector();
	JmyTcpConnector(io_service& service, const ip::tcp::endpoint& ep);
	~JmyTcpConnector();

	bool init(io_service& service);
	void close();
	void destroy();
	void reset();

	JmyConnectorState getState() const { return state_; }

	bool loadConfig(const JmyConnectorConfig& conf);
	void async_connect(const char* ip, short port);
	void connect(const char* ip, short port);
	void start();
	int send(int msg_id, const char* data, unsigned int len);
	int run();

	std::shared_ptr<ip::tcp::socket> getSock() { return sock_; }

private:
	int handle_send();

private:
	std::shared_ptr<ip::tcp::socket> sock_;
	ip::tcp::endpoint ep_;
	JmyConnectorState state_;
	JmySessionBuffer recv_buff_;
	JmySessionBuffer send_buff_;
	JmyDataHandler handler_;
	JmyConnectorConfig conf_;
	bool sending_;
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
