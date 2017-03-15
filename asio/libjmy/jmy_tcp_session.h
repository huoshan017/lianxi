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

	ip::tcp::socket& getSock() { return sock_; }
	int getId() const { return id_; }
	void* getUnusedData() const { return unused_data_; }
	void setUnusedData(void* data) { unused_data_ = data; }

private:
	int handle_recv();
	int handle_send();

private:
	int id_;
	ip::tcp::socket sock_;
	std::shared_ptr<JmyTcpSessionMgr> session_mgr_;
	std::shared_ptr<JmyDataHandler> handler_;
	JmyDoubleSessionBuffer recv_buff_;
	JmyDoubleSessionBuffer send_buff_;
	JmySessionBufferList send_buff_list_;
	bool sending_;
	void* unused_data_;
	bool use_send_list_;
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

