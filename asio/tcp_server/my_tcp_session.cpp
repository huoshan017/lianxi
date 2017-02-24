#include "my_tcp_session.h"
#include <iostream>

MyTcpSession::MyTcpSession(io_service& service) : id_(0), sock_(service), sending_(false)
{
}

MyTcpSession::~MyTcpSession()
{
}

bool MyTcpSession::init(const MySessionConfig& conf, int id, std::shared_ptr<MyDataHandler> handler)
{
	if (!recv_buff_.init(conf.recv_buff_min)) return false;
	if (!send_buff_.init(conf.send_buff_min)) return false;
	id_ = id;
	handler_ = handler;
	return true;
}

void MyTcpSession::close()
{
	recv_buff_.destroy();
	send_buff_.destroy();
	sock_.close();
}

void MyTcpSession::reset()
{
	recv_buff_.clear();
	send_buff_.clear();
	sock_.close();
}

void MyTcpSession::start()
{
	if (id_ == 0) {
		std::cout << "not init" << std::endl;
		return;
	}
	sock_.async_read_some(boost::asio::buffer(recv_buff_.getWriteBuff(), recv_buff_.getWriteLen()),
		[this](const boost::system::error_code& err, size_t bytes_transferred) {
			if (!err) {
				if (bytes_transferred > 0) {
					recv_buff_.writeLen(bytes_transferred);
				}

				int nread = handle_read();
				if (nread < 0) {
					std::cout << "MyTcpSession::start handle_read failed" << std::endl;
					return;
				}
				start();
			} else {
				int ev = err.value();
				if (ev == 10053 || ev == 10054) {

				}
				std::cout << "MyTcpSession::start  read some data failed, err: " << err << std::endl;
			}
		} );
}

int MyTcpSession::handle_read()
{
	char* data = recv_buff_.getReadBuff();
	int len = recv_buff_.getReadLen();
	int res = handler_->processData(data, len, getId());
	if (res > 0) {
		recv_buff_.readLen(len);
	} else if (res == 0) {
		// not enough length to get, move left data to front
		int left_write = recv_buff_.getWriteLen();
		if (left_write == 0) {
			recv_buff_.moveDataToFront();
		}
	}
	return res;
}

int MyTcpSession::run()
{
	handle_write();
	return 1;
}

int MyTcpSession::send(const char* data, unsigned int len)
{
	bool res = send_buff_.writeData(data, len);
	if (!res) {
		if (handle_write() == 0)
			return res;
		res = send_buff_.writeData(data, len);
	} else {
		handle_write();
	}
	return 0;
}

int MyTcpSession::handle_write()
{
	if (send_buff_.getReadLen() == 0)
		return 0;

	if (sending_)
		return 0;

	sock_.async_send(boost::asio::buffer(send_buff_.getReadBuff(), send_buff_.getReadLen()),
			[this](const boost::system::error_code& err, size_t bytes_transferred){
				if (!err) {
					send_buff_.readLen(bytes_transferred);
				} else {
					std::cout << "MyTcpSession::handle_write  async_send error: " << err << std::endl;
					return;
				}
				sending_ = false;
			});
	sending_ = true;
	return 1;
}

/**
 * MyTcpSessionMgr
 */
MyTcpSessionMgr::MyTcpSessionMgr() : curr_id_(0), max_session_size_(5000)
{
}

MyTcpSessionMgr::~MyTcpSessionMgr()
{
	clear();
}

bool MyTcpSessionMgr::init(int max_session_size, io_service& service)
{
	if (max_session_size > max_session_size_)
		max_session_size_ = max_session_size;
	int i = 0;
	for (; i<max_session_size_; ++i) {
		free_session_list_.push_back(new MyTcpSession(service));
	}
	return true;
}

void MyTcpSessionMgr::clear()
{
	std::unordered_map<int, MyTcpSession*>::iterator it = used_session_map_.begin();
	for (; it!=used_session_map_.end();++it) {
		if (it->second) {
			delete it->second;
		}
	}
	used_session_map_.clear();
	std::list<MyTcpSession*>::iterator lit = free_session_list_.begin();
	for (; lit!=free_session_list_.end(); ++lit) {
		MyTcpSession* session = *lit;
		if (session) {
			delete session;
		}
	}
	free_session_list_.clear();
}

MyTcpSession* MyTcpSessionMgr::getOneSession(const MySessionConfig& conf, std::shared_ptr<MyDataHandler> handler)
{
	if (free_session_list_.size() == 0)
		return NULL;

	MyTcpSession* session = free_session_list_.front();
	if (!session)
		return NULL;

	curr_id_ += 1;
	if (curr_id_ > max_session_size_*2)
		curr_id_ = 1;

	if (!session->init(conf, curr_id_, handler))
		return NULL;

	free_session_list_.pop_front();
	used_session_map_.insert(std::make_pair(curr_id_, session));
	return session;	
}

int MyTcpSessionMgr::freeSession(MyTcpSession* session)
{
	if (!session) return 0;
	session->reset();
	return freeSessionById(session->getId());
}

int MyTcpSessionMgr::freeSessionById(int id)
{
	MyTcpSession* session = getSessionById(id);
	if (!session)
		return 0;
	free_session_list_.push_back(session);
	used_session_map_.erase(id);
	return 1;
}

MyTcpSession* MyTcpSessionMgr::getSessionById(int id)
{
	std::unordered_map<int, MyTcpSession*>::iterator it = used_session_map_.find(id);
	if (it == used_session_map_.end()) {
		return NULL;
	}
	if (!it->second) {
		used_session_map_.erase(it);
		return NULL;
	}
	return it->second;
}
