#pragma once

#if USE_CONNECTOR_AND_SESSION

#include <unordered_map>
#include <set>
#include <mutex>
#include <boost/asio.hpp>
#include "jmy_datatype.h"
#include "jmy_session_buffer.h"
#include "jmy_data_handler.h"
#include "jmy_net_tool.h"

using namespace boost::asio;

enum JmyConnectorState {
	CONNECTOR_STATE_NOT_CONNECT = 0,
	CONNECTOR_STATE_CONNECTING = 1,
	CONNECTOR_STATE_CONNECTED = 2,
	CONNECTOR_STATE_DISCONNECTING = 3,
	CONNECTOR_STATE_DISCONNECT = 4,
};

class JmyTcpConnector 
{
public:
	JmyTcpConnector(io_service& service, JmyTcpConnectorMgr& mgr);
	JmyTcpConnector(io_service& service, JmyTcpConnectorMgr& mgr, const ip::tcp::endpoint& ep);
	~JmyTcpConnector();

	void close();
	void destroy();
	void reset();

	bool loadConfig(const JmyConnectorConfig& conf);
	void asynConnect(const char* ip, short port);
	void connect(const char* ip, short port);
	void start();
	bool isStarting() const { return starting_; }
	int send(int msg_id, const char* data, unsigned int len);
	int run();

	JmyConnectorState getState() const { return state_; }
	ip::tcp::socket& getSock() { return sock_; }
	std::string getIp() const { return ep_.address().to_string(); }
	unsigned short getPort() const { return ep_.port(); }
	int getId() const { return conf_.conn_id; }
	void* getUnusedData() const { return unused_data_; }
	void setUnusedData(void* data) { unused_data_ = data; }
#if USE_CONN_PROTO
	void setConnResInfo(JmyConnResInfo& info) { total_reconn_info_.conn_info = info; }
#endif
private:
	int handle_send();
	int check_reconn_info_for_recv(unsigned short recv_count);
	int check_reconn_info_for_send();

private:
	JmyTcpConnectorMgr& mgr_;
	ip::tcp::socket sock_;
	ip::tcp::endpoint ep_;
	JmyConnectorState state_;
	JmySessionBuffer recv_buff_, send_buff_;
	JmySessionBufferList send_buff_list_;
	bool use_send_list_;
	JmyDataHandler handler_;
	JmyConnectorConfig conf_;
	bool starting_;
	bool sending_;
	std::chrono::system_clock::time_point last_tick_;
	JmyTotalReconnInfo total_reconn_info_;

	JmyNetTool tool_;
	void* unused_data_;

	friend class JmyTcpConnectorMgr;
};

class JmyTcpConnectorMgr 
{
public:
	JmyTcpConnectorMgr();
	~JmyTcpConnectorMgr();
	void clear();

	void init(bool use_multi_threads = false, bool use_auto_id = false);
	void useMultiThreads(bool enable = false) { use_multi_threads_ = enable; }
	void useAutoId(bool enable = false) { use_auto_id_ = enable; }
	JmyTcpConnector* newConnector(io_service& service);
	JmyTcpConnector* newConnector(io_service& service, int id);
	JmyTcpConnector* get(int id);
	bool check(int id, bool locked = false);
	bool insert(int id, JmyTcpConnector* conn);
	int insert(JmyTcpConnector* conn);
	bool remove(int id);
	bool remove(JmyTcpConnector* conn);

private:
	enum { MaxId = 99999, };
	std::unordered_map<int, JmyTcpConnector*> conns_;
	bool use_auto_id_;
	bool use_multi_threads_;
	std::mutex mtx_;
	int curr_id_;
};

class JmyTcpMultiConnectors
{
public:
	JmyTcpMultiConnectors(io_service& service, int max_count = 5000);
	JmyTcpMultiConnectors(io_service& service, const ip::tcp::endpoint& ep, int max_count = 5000);
	~JmyTcpMultiConnectors();

	void close();
	void destroy();
	void reset();

	bool loadConfig(const JmyMultiConnectorsConfig& conf);
	JmyTcpConnector* start(const char* ip, short port);
	int send(int connector_id, int msg_id, const char* data, unsigned int len);
	int run(int connector_id);
	int startInturn(int count, const char* ip, short port);
	int sendInturn(int msg_id, const char* data, unsigned int len);
	int runInturn();

	JmyTcpConnector* getConnector(int connector_id);
	JmyConnectorState getState(int connector_id);

private:
	io_service& service_;
	int max_count_;
	std::set<int> used_ids_;
	std::list<int> free_ids_;
	ip::tcp::endpoint ep_;
	JmyMultiConnectorsConfig conf_;
	JmyTcpConnectorMgr mgr_;
};
#endif
