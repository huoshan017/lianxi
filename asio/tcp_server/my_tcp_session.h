#pragma once

#include <list>
#include <unordered_map>
#include <boost/asio.hpp>
#include "my_datatype.h"
#include "my_session_buffer.h"
#include "my_singleton.hpp"
#include "my_data_handler.h"

using namespace boost::asio;

class MyTcpSession : public std::enable_shared_from_this<MyTcpSession>
{
public:
	MyTcpSession(io_service& service);
	~MyTcpSession();

	bool init(const MySessionConfig& conf, int id, std::shared_ptr<MyDataHandler> handler);
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
	std::shared_ptr<MyDataHandler> handler_;
	MySessionBuffer recv_buff_;
	MySessionBuffer send_buff_;
	bool sending_;
};

class MyTcpSessionMgr : public MySingleton<MyTcpSessionMgr>
{
public:
	MyTcpSessionMgr();
	~MyTcpSessionMgr();

	bool init(int max_session_size, io_service& service);
	void clear();
	
	MyTcpSession* getOneSession(const MySessionConfig& conf, std::shared_ptr<MyDataHandler> handler);
	int freeSession(MyTcpSession*);
	int freeSessionById(int id);
	MyTcpSession* getSessionById(int id);
	int run();

private:
	std::unordered_map<int, MyTcpSession*> used_session_map_;
	std::list<MyTcpSession*> free_session_list_;
	int curr_id_;
	int max_session_size_;
};

#define SESSION_MGR (MyTcpSessionMgr::getInstance())
