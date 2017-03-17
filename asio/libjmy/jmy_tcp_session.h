#pragma once

#include <list>
#include <unordered_map>
#include <boost/asio.hpp>
#include "jmy_datatype.h"
#include "jmy_session_buffer.h"

using namespace boost::asio;

class JmySessionBufferPool;
class JmyDataHandler;
class JmyTcpSessionMgr;

class JmyTcpSession
{
public:
	JmyTcpSession(io_service& service);
	~JmyTcpSession();

	bool init(int id,
			std::shared_ptr<JmyTcpSessionMgr> session_mgr,
			std::shared_ptr<JmySessionBufferPool> pool,
			std::shared_ptr<JmyDataHandler> handler,
			bool use_send_list = false);
	void close();
	void reset();
	void start();
	int run();

	int send(int msg_id, const char* data, unsigned int len);
	int sendAck(JmyAckInfo*);
	int sendHeartbeat();
	void checkAck(JmyAckInfo& info);
#if USE_CONN_PROTO
	int sendConnRes(JmyConnResInfo*);
	int sendReconnRes(JmyConnResInfo*);
	int checkReconn(JmyConnResInfo*);
#endif
	ip::tcp::socket& getSock() { return sock_; }
	int getId() const { return id_; }
	void* getUnusedData() const { return unused_data_; }
	void setUnusedData(void* data) { unused_data_ = data; }
#if USE_CONN_PROTO
	void setConnResInfo(JmyConnResInfo& info) { total_reconn_info_.conn_info = info; }
#endif
private:
	int handle_recv();
	int handle_send();

private:
	int id_;
	ip::tcp::socket sock_;
	std::shared_ptr<JmyTcpSessionMgr> session_mgr_;		// session manager shared pointer
	std::shared_ptr<JmyDataHandler> handler_;			// data handler
	JmySimpleBuffer conn_send_buff_, conn_recv_buff_;	// conn buffer
	JmyDoubleSessionBuffer recv_buff_, send_buff_;		// data buffer
	JmySessionBufferList send_buff_list_;				// send buffer list
	bool use_send_list_;								// is use send buffer list
	bool sending_;										// is sending data
	JmyTotalReconnInfo total_reconn_info_;				// hold total reconn info
	void* unused_data_;									// extra data when need to use
};

class JmyTcpSessionMgr : public std::enable_shared_from_this<JmyTcpSessionMgr>
{
public:
	JmyTcpSessionMgr();
	~JmyTcpSessionMgr();

	bool init(int max_session_size, io_service& service);
	void clear();
	
	JmyTcpSession* getOneSession(std::shared_ptr<JmySessionBufferPool> pool,
								std::shared_ptr<JmyDataHandler> handler,
								bool use_send_list = false);
	int freeSession(JmyTcpSession*);
	int freeSessionById(int id);
	JmyTcpSession* getSessionById(int id);
	int run();

private:
	std::unordered_map<int, JmyTcpSession*> used_session_map_;
	std::list<JmyTcpSession*> free_session_list_;
	int curr_id_;
	int max_session_size_;
};

