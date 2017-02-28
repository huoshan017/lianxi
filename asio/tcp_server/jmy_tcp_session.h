#pragma once

#include <list>
#include <unordered_map>
#include <boost/asio.hpp>
#include "jmy_datatype.h"
#include "jmy_session_buffer.h"
#include "jmy_singleton.hpp"
#include "jmy_data_handler.h"

using namespace boost::asio;

class JmyTcpSession : public std::enable_shared_from_this<JmyTcpSession>
{
public:
	JmyTcpSession(io_service& service);
	~JmyTcpSession();

	bool init(const JmySessionConfig& conf, int id, std::shared_ptr<JmyDataHandler> handler);
	void close();
	void reset();
	void start();
	int run();
	int send(const char* data, unsigned int len);
	ip::tcp::socket& getSock() { return sock_; }
	int getId() const { return id_; }

private:
	int handle_read();
	int handle_write();

private:
	int id_;
	ip::tcp::socket sock_;
	std::shared_ptr<JmyDataHandler> handler_;
	JmyDoubleSessionBuffer recv_buff_;
	JmyDoubleSessionBuffer send_buff_;
	bool sending_;
};

class JmyTcpSessionMgr : public JmySingleton<JmyTcpSessionMgr>
{
public:
	JmyTcpSessionMgr();
	~JmyTcpSessionMgr();

	bool init(int max_session_size, io_service& service);
	void clear();
	
	JmyTcpSession* getOneSession(const JmySessionConfig& conf, std::shared_ptr<JmyDataHandler> handler);
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

#define SESSION_MGR (JmyTcpSessionMgr::getInstance())
