#include "jmy_tcp_session.h"
#include "jmy_data_handler.h"
#include <iostream>

JmyTcpSession::JmyTcpSession(io_service& service) : id_(0), sock_(service), sending_(false)
{
}

JmyTcpSession::~JmyTcpSession()
{
	close();
}

bool JmyTcpSession::init(int id,
		std::shared_ptr<JmyTcpSessionMgr> session_mgr,
		std::shared_ptr<JmySessionBufferPool> pool,
		std::shared_ptr<JmyDataHandler> handler)
{
	if (!recv_buff_.init(pool, SESSION_BUFFER_TYPE_RECV)) return false;
	if (!send_buff_.init(pool, SESSION_BUFFER_TYPE_SEND)) return false;
	id_ = id;
	session_mgr_ = session_mgr;
	handler_ = handler;
	return true;
}

void JmyTcpSession::close()
{
	recv_buff_.destroy();
	send_buff_.destroy();
	sock_.close();
}

void JmyTcpSession::reset()
{
	recv_buff_.clear();
	send_buff_.clear();
	sending_ = false;
}

void JmyTcpSession::start()
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

				int nread = handle_recv();
				if (nread < 0) {
					std::cout << "JmyTcpSession::start handle_read failed" << std::endl;
					return;
				}
				start();
			} else {
				int ev = err.value();
				if (ev == 10053 || ev == 10054) {

				}
				std::cout << "JmyTcpSession::start  read some data failed, err: " << err << std::endl;
			}
		} );
}

int JmyTcpSession::handle_recv()
{
	int res = handler_->processData<JmyDoubleSessionBuffer>(&recv_buff_, getId(), session_mgr_);
	return res;
}

int JmyTcpSession::send(int msg_id, const char* data, unsigned int len)
{
	int res = handler_->writeData(&send_buff_, msg_id, data, len);
	if (res < 0) {
		std::cout << "JmyTcpSession::send  write data length(" << len << ") failed" << std::endl;
		return -1;
	}
	std::cout << "JmyTcpSession::send  session " << getId() << " write length " << len << " of data to send buffer" << std::endl;
	return len;
}

int JmyTcpSession::handle_send()
{
	if (send_buff_.getReadLen() == 0)
		return 0;

	if (sending_)
		return 0;

	sock_.async_write_some(boost::asio::buffer(send_buff_.getReadBuff(), send_buff_.getReadLen()),
			[this](const boost::system::error_code& err, size_t bytes_transferred){
				if (!err) {
					send_buff_.readLen(bytes_transferred);
					std::cout << "JmyTcpSession::handle_send  session << " << getId() << " sent " << bytes_transferred << " bytes data" << std::endl;
				} else {
					std::cout << "JmyTcpSession::handle_send  async_send error: " << err << std::endl;
					return;
				}
				sending_ = false;
			});
	sending_ = true;
	return 1;
}

int JmyTcpSession::run()
{
	return handle_send();
}

/**
 * JmyTcpSessionMgr
 */
JmyTcpSessionMgr::JmyTcpSessionMgr() : curr_id_(0), max_session_size_(5000)
{
}

JmyTcpSessionMgr::~JmyTcpSessionMgr()
{
	clear();
}

bool JmyTcpSessionMgr::init(int max_session_size, io_service& service)
{
	if (max_session_size > max_session_size_)
		max_session_size_ = max_session_size;
	int i = 0;
	for (; i<max_session_size_; ++i) {
		free_session_list_.push_back(new JmyTcpSession(service));
	}
	return true;
}

void JmyTcpSessionMgr::clear()
{
	std::unordered_map<int, JmyTcpSession*>::iterator it = used_session_map_.begin();
	for (; it!=used_session_map_.end();++it) {
		if (it->second) {
			delete it->second;
		}
	}
	used_session_map_.clear();
	std::list<JmyTcpSession*>::iterator lit = free_session_list_.begin();
	for (; lit!=free_session_list_.end(); ++lit) {
		JmyTcpSession* session = *lit;
		if (session) {
			delete session;
		}
	}
	free_session_list_.clear();
}

JmyTcpSession* JmyTcpSessionMgr::getOneSession(std::shared_ptr<JmySessionBufferPool> pool, std::shared_ptr<JmyDataHandler> handler)
{
	if (free_session_list_.size() == 0)
		return NULL;

	JmyTcpSession* session = free_session_list_.front();
	if (!session)
		return NULL;

	curr_id_ += 1;
	if (curr_id_ > max_session_size_*2)
		curr_id_ = 1;

	if (!session->init(curr_id_, shared_from_this(), pool, handler))
		return NULL;

	free_session_list_.pop_front();
	used_session_map_.insert(std::make_pair(curr_id_, session));
	return session;	
}

int JmyTcpSessionMgr::freeSession(JmyTcpSession* session)
{
	if (!session) return 0;
	session->reset();
	return freeSessionById(session->getId());
}

int JmyTcpSessionMgr::freeSessionById(int id)
{
	JmyTcpSession* session = getSessionById(id);
	if (!session)
		return 0;
	free_session_list_.push_back(session);
	used_session_map_.erase(id);
	return 1;
}

JmyTcpSession* JmyTcpSessionMgr::getSessionById(int id)
{
	std::unordered_map<int, JmyTcpSession*>::iterator it = used_session_map_.find(id);
	if (it == used_session_map_.end()) {
		return NULL;
	}
	if (!it->second) {
		used_session_map_.erase(it);
		return NULL;
	}
	return it->second;
}

int JmyTcpSessionMgr::run()
{
	for (auto s: used_session_map_) {
		if (s.second) {
			if (s.second->run() < 0) {
				std::cout << "JmyTcpSessionMgr::run, session: " << s.second->getId() << " run failed" << std::endl; 
			}
		}
	}
	return 0;
}
