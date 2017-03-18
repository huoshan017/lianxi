#pragma once

#include <chrono>
#include <boost/asio.hpp>
#include "jmy_datatype.h"
#include "jmy_data_handler.h"
#include "jmy_connection_buffer.h"

using namespace boost::asio;

class JmyTcpConnection
{
public:
	JmyTcpConnection(io_service& service, JmyConnType conn_type);
	~JmyTcpConnection();
	void close();
	void destroy();
	void reset();

	void asynConnect(const char* ip, short port);
	void connect(const char* ip, short port);
	void start();
	int send(int msg_id, const char* data, unsigned int len);
	int run();

	void setId(int id) { id_ = id; }
	int getId() const { return id_; }
	ip::tcp::socket& getSock() { return sock_; }
	JmyConnType getConnType() const { return conn_type_; }
	JmyConnState getConnState() const { return state_; }
	bool isDisconnect() const { return state_ == JMY_CONN_STATE_DISCONNECTED; } 
	void setDataHandler(std::shared_ptr<JmyDataHandler> handler) { handler_ = handler; }
	void setBuffer(std::shared_ptr<JmyConnectionBuffer> buffer) { buffer_ = buffer; }
	void* getUnusedData() const { return unused_data_; }
	void setUnusedData(void* data) { unused_data_ = data; }

protected:
	int sendAck(JmyAckInfo*);
	int sendHeartbeat();
	int sendDisconnect();
	int sendDisconnectAck();
	int handleAck();
	int handleHeartbeat();
	int handleDisconnect();
	int handleDisconnectAck();
	int handle_recv();
	int handle_send();

protected:
	int id_;
	ip::tcp::socket sock_;
	ip::tcp::endpoint ep_;
	JmyConnType conn_type_;										// connection type
	JmyConnState state_;										// connection state
	bool sending_data_;											// is sending data
	std::chrono::system_clock::time_point active_close_start_;	// active close time start
	std::shared_ptr<JmyDataHandler> handler_;					// data handler
	std::shared_ptr<JmyConnectionBuffer> buffer_;				// recv and send buffer
	JmyTotalReconnInfo total_reconn_info_;						// hold total reconn info
	void* unused_data_;											// extra data when need to use
};

class JmyTcpConnectionMgr
{
public:
	JmyTcpConnectionMgr();
	~JmyTcpConnectionMgr();
	void clear();
	void init(int size);
	JmyTcpConnection* getFree(int id);
	JmyTcpConnection* get(int id);
	bool free(JmyTcpConnection* conn);
	int usedRun();

private:
	JmyConnectionBufferMgr buffer_mgr_;
	std::unordered_map<int, JmyTcpConnection*> used_map_;
	std::list<JmyTcpConnection*> free_list_;
	int size_;
};

