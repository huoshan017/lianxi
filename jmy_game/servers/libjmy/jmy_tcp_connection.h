#pragma once

#include <chrono>
#include <boost/asio.hpp>
#include "jmy_const.h"
#include "jmy_datatype.h"

#if USE_NET_PROTO2
#include "jmy_data_handler2.h"
#else
#include "jmy_data_handler.h"
#endif

#include "jmy_connection_buffer.h"
#include "jmy_event_handler.h"

using namespace boost::asio;

class JmyTcpConnectionMgr;

enum {
	CONN_CLOSE_REASON_NORMAL = 0,
	CONN_CLOSE_REASON_READ_DATA_FAILED = 1,
	CONN_CLOSE_REASON_WRITE_DATA_FAILED = 2,
	CONN_CLOSE_REASON_READ_MSG_HEAD_NOT_ENOUGH = 3,
	CONN_CLOSE_REASON_READ_MSG_DATA_NOT_ENOUGH = 4,
	CONN_CLOSE_REASON_RECV_CLOSE_EVENT = 5,
	CONN_CLOSE_REASON_TIMEOUT = 6,
};

class JmyTcpConnection
{
public:
	JmyTcpConnection(io_service& service, JmyTcpConnectionMgr& mgr, JmyConnType conn_type = JMY_CONN_TYPE_NONE);
	~JmyTcpConnection();
	void close();
	void force_close(int reason = CONN_CLOSE_REASON_NORMAL);
	void destroy();
	void reset();

	void asynConnect(const char* ip, short port);
	bool connect(const char* ip, short port);
	void start();
	int send(int msg_id, const char* data, unsigned int len);
	int send(int user_id, int msg_id, const char* data, unsigned short len);
	int run();

	// geter and seter
	void setId(int id) { id_ = id; }
	int getId() const { return id_; }
	ip::tcp::socket& getSock() { return sock_; }
	JmyConnType getConnType() const { return conn_type_; }
	JmyConnState getConnState() const { return state_; }
	bool isDisconnect() const { return state_ == JMY_CONN_STATE_DISCONNECTED; } 
	bool isConnected() const { return state_ == JMY_CONN_STATE_DISCONNECTED; }
	// set data handler
#if USE_NET_PROTO2
	void setDataHandler(std::shared_ptr<JmyDataHandler2> handler) { data_handler_ = handler; }
#else
	void setDataHandler(std::shared_ptr<JmyDataHandler> handler) { data_handler_ = handler; }
#endif
	// set event handler
	void setEventHandler(std::shared_ptr<JmyEventHandlerManager> handler) { event_handler_ = handler; }
	std::shared_ptr<JmyConnectionBuffer> getBuffer() const { return buffer_; }
	// set buffer
	void setBuffer(std::shared_ptr<JmyConnectionBuffer> buffer) { buffer_ = buffer; }
	JmyTcpConnectionMgr& getConnectionMgr() { return mgr_; }

	int handleHeartbeat();
	int handleDisconnect();
	int handleDisconnectAck();

protected:
	void start_recv();
#if USE_NET_PROTO2
	void recv_packet(JmyPacketType2 type, unsigned short pack_len);
#endif
	int start_send();
	void buffer_send();
	void buffer_list_send();
	int sendHeartbeat();
	int sendDisconnect();
	int sendDisconnectAck();
	int handle_recv();
	int handle_send();
	int handle_event(int event_id, long param);
	int handle_packet_type(JmyPacketType2 packet_type);
	int handle_recv_error(const boost::system::error_code& err);

protected:
	int id_;
	io_service::strand strand_;		
	ip::tcp::socket sock_;
	ip::tcp::endpoint ep_;
	JmyTcpConnectionMgr& mgr_;
	JmyConnType conn_type_;										// connection type
	JmyConnState state_;										// connection state
	bool sending_data_;											// is sending data
	short buff_send_count_;
	std::chrono::system_clock::time_point active_close_start_;	// active close time start
#if USE_NET_PROTO2
	std::shared_ptr<JmyDataHandler2> data_handler_;
#else
	std::shared_ptr<JmyDataHandler> data_handler_;				// data handler
#endif
	std::shared_ptr<JmyEventHandlerManager> event_handler_;		// event handler
	std::shared_ptr<JmyConnectionBuffer> buffer_;				// recv and send buffer
	JmyEventInfo event_info_;
	std::chrono::system_clock::time_point last_run_tick_;
	char tmp_[JMY_PACKET2_LEN_HEAD+JMY_PACKET2_LEN_TYPE];
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
	const JmyConnectionConfig& getConf() const { return conf_; }

private:
	io_service& service_;
	JmyConnectionConfig conf_;
	std::unordered_map<int, JmyTcpConnection*> used_map_;
	std::list<JmyTcpConnection*> free_list_;
	int size_;
};
