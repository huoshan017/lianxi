#pragma once

#include <chrono>
#include <boost/asio.hpp>
#if USE_COROUTINE
#include <boost/asio/yield.hpp>
#endif
#include "jmy_const.h"
#include "jmy_datatype.h"
#include "jmy_data_handler.h"
#include "jmy_connection_buffer.h"
#include "jmy_event_handler.h"

using namespace boost::asio;

class JmyTcpConnectionMgr;

class JmyTcpConnection
{
public:
	JmyTcpConnection(io_service& service, JmyTcpConnectionMgr& mgr, JmyConnType conn_type = JMY_CONN_TYPE_NONE);
	~JmyTcpConnection();
	void close();
	void force_close();
	void destroy();
	void reset();

	void asynConnect(const char* ip, short port);
	bool connect(const char* ip, short port);
	void start();
	int send(int msg_id, const char* data, unsigned int len);
	int send(int user_id, int msg_id, const char* data, unsigned short len);
	int run();
#if USE_COROUTINE
	// start coroutine function
	int go();

	class recv_coro : public boost::asio::coroutine {
	public:
		recv_coro(JmyTcpConnection* conn);
		~recv_coro();
		void operator()(const boost::system::error_code ec, std::size_t bytes_transferred);
	private:
		JmyTcpConnection* conn_;
	};
	friend class recv_coro;

	class send_coro : public boost::asio::coroutine {
	public:
		send_coro(JmyTcpConnection* conn);
		~send_coro();
		void operator()(const boost::system::error_code ec, std::size_t bytes_transferred);
	private:
		JmyTcpConnection* conn_;
	};
	friend class send_coro;
#endif

	// geter and seter
	void setId(int id) { id_ = id; }
	int getId() const { return id_; }
	ip::tcp::socket& getSock() { return sock_; }
	JmyConnType getConnType() const { return conn_type_; }
	JmyConnState getConnState() const { return state_; }
	bool isDisconnect() const { return state_ == JMY_CONN_STATE_DISCONNECTED; } 
	bool isConnected() const { return state_ == JMY_CONN_STATE_DISCONNECTED; }
	// set data handler
	void setDataHandler(std::shared_ptr<JmyDataHandler> handler) { data_handler_ = handler; }
	// set event handler
	void setEventHandler(std::shared_ptr<JmyEventHandlerManager> handler) { event_handler_ = handler; }
	std::shared_ptr<JmyConnectionBuffer> getBuffer() const { return buffer_; }
	// set buffer
	void setBuffer(std::shared_ptr<JmyConnectionBuffer> buffer) { buffer_ = buffer; }
	JmyTcpConnectionMgr& getConnectionMgr() { return mgr_; }

	//int handleAck(JmyAckInfo*);
	int handleHeartbeat();
	int handleDisconnect();
	int handleDisconnectAck();

protected:
	void start_recv();
	int start_send();
	void buffer_send();
	void buffer_list_send();
	//int sendAck(JmyAckInfo*);
	int sendHeartbeat();
	int sendDisconnect();
	int sendDisconnectAck();
	int handle_recv();
	int handle_send();
	int handle_event(int event_id, long param);

protected:
	int id_;
	io_service::strand strand_;		
#if USE_COROUTINE
	bool coroutine_running_;
	boost::asio::coroutine recv_coro_;
	boost::asio::coroutine send_coro_;
#endif
	ip::tcp::socket sock_;
	ip::tcp::endpoint ep_;
	JmyTcpConnectionMgr& mgr_;
	JmyConnType conn_type_;										// connection type
	JmyConnState state_;										// connection state
	bool sending_data_;											// is sending data
	std::chrono::system_clock::time_point active_close_start_;	// active close time start
	std::shared_ptr<JmyDataHandler> data_handler_;				// data handler
	std::shared_ptr<JmyEventHandlerManager> event_handler_;		// event handler
	std::shared_ptr<JmyConnectionBuffer> buffer_;				// recv and send buffer
	JmyEventInfo event_info_;
	std::chrono::system_clock::time_point last_run_tick_;
};

class JmyTcpConnectionMgr
{
public:
	JmyTcpConnectionMgr(io_service& service);
	~JmyTcpConnectionMgr();
	void clear();
	void init(int conn_size, JmyConnType conn_type);
	void init(int conn_size, JmyConnType conn_type, const JmyConnectionConfig& conf);
	JmyTcpConnection* getFree(int id);
	JmyTcpConnection* get(int id);
	bool free(JmyTcpConnection* conn);
#if 0
	int usedRun();
#endif

	const JmyConnectionConfig& getConf() const { return conf_; }

private:
	io_service& service_;
	JmyConnectionConfig conf_;
	std::unordered_map<int, JmyTcpConnection*> used_map_;
	std::list<JmyTcpConnection*> free_list_;
	int size_;
};

