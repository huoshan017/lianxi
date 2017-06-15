#if USE_CONNECTOR_AND_SESSION
#include "jmy_tcp_session.h"
#include "jmy_data_handler.h"
#include "jmy_log.h"
#include <thread>
#include <chrono>
#include <memory>

JmyTcpSession::JmyTcpSession(io_service& service) : id_(0), sock_(service), use_send_list_(false), sending_(false), unused_data_(NULL)
{
	std::memset(&total_reconn_info_, 0, sizeof(total_reconn_info_));
}

JmyTcpSession::~JmyTcpSession()
{
	close();
}

bool JmyTcpSession::init(int id,
		std::shared_ptr<JmyTcpSessionMgr> session_mgr,
		std::shared_ptr<JmySessionBufferPool> pool,
		std::shared_ptr<JmyDataHandler> handler,
		bool use_send_list)
{
	if (!recv_buff_.init(pool, SESSION_BUFFER_TYPE_RECV)) return false;
	if (!send_buff_.init(pool, SESSION_BUFFER_TYPE_SEND)) return false;
	if (use_send_list) {
		send_buff_list_.init(0, 0);
	}
	id_ = id;
	session_mgr_ = session_mgr;
	handler_ = handler;
	use_send_list_ = use_send_list;
	return true;
}

void JmyTcpSession::close()
{
	recv_buff_.destroy();
	send_buff_.destroy();
	if (use_send_list_) {
		send_buff_list_.reset();
	}
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
		LibJmyLogError("not init");
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
					LibJmyLogError("handle_recv return ");
					return;
				}
				if (bytes_transferred == 0) {
					std::this_thread::sleep_for(std::chrono::milliseconds(10));	
				}
				start();
			} else {
				int ev = err.value();
				if (ev == 10053 || ev == 10054) {

				}
				sock_.close();
				session_mgr_->freeSessionById(getId());
				LibJmyLogError("read some data failed, err: %d, session_id: %d", err.value(), getId());
			}
		} );
}

int JmyTcpSession::handle_recv()
{
	int nread = handler_->processData(recv_buff_, getId(), (void*)session_mgr_.get());	
	return nread;
}

int JmyTcpSession::send(int msg_id, const char* data, unsigned int len)
{
	int res = 0;
	if (use_send_list_) {
		res = handler_->writeData(&send_buff_list_, msg_id, data, len);
	} else {
		res = handler_->writeData(&send_buff_, msg_id, data, len);
	}
	if (res < 0) {
		LibJmyLogError("write data length(%d) failed", len);
		return -1;
	}
	return len;
}

int JmyTcpSession::sendAck(JmyAckInfo* info)
{
	int res = 0;
	if (use_send_list_) {
		res = handler_->writeAck(&send_buff_list_, info->ack_count, info->curr_id);
	} else {
		res = handler_->writeAck(&send_buff_, info->ack_count, info->curr_id);
	}
	if (res < 0) {
		LibJmyLogError("write ack failed");
		return -1;
	}
	return res;
}

int JmyTcpSession::sendHeartbeat()
{
	int res = 0;
	if (use_send_list_) {
		res = handler_->writeHeartbeat(&send_buff_list_);
	} else {
		res = handler_->writeHeartbeat(&send_buff_);
	}
	if (res < 0) {
		LibJmyLogError("write heart beat failed");
		return -1;
	}
	return res;
}

#if USE_CONN_PROTO
// send to JmyTcpConnector or client
int JmyTcpSession::sendConnRes(JmyConnResInfo* info)
{
	conn_send_buff_.reset();
	int res = handler_->writeConnRes(&conn_send_buff_, info->conn_id, info->session_str);
	if (res < 0) {
		LibJmyLogError("write ack conn failed");
		return -1;
	}
	return res;
}

// send to JmyTcpConnector or client
int JmyTcpSession::sendReconnRes(JmyConnResInfo* info)
{
	conn_send_buff_.reset();
	int res = handler_->writeReconnRes(&conn_send_buff_, info->conn_id, info->session_str);
	if (res < 0) {
		LibJmyLogError("write ack reconn failed");
		return -1;
	}
	return res;
}

int JmyTcpSession::checkReconn(JmyConnResInfo* info)
{
	if (info->conn_id != total_reconn_info_.conn_info.conn_id ||
		info->session_str_len != total_reconn_info_.conn_info.session_str_len ||
		std::memcmp(info->session_str, total_reconn_info_.conn_info.session_str, info->session_str_len) != 0)  {
		LibJmyLogError("verify conn failed, conn_id(%d, %d) session_str(%s, %s)",
				info->conn_id, total_reconn_info_.conn_info.conn_id,
				info->session_str, total_reconn_info_.conn_info.session_str);
		return -1;
	}

	return 0;
}
#endif

int JmyTcpSession::handle_send()
{
	if (send_buff_.getReadLen() == 0)
		return 0;

	if (sending_)
		return 0;

	if (use_send_list_) {
		sock_.async_write_some(boost::asio::buffer(send_buff_list_.getReadBuff(), send_buff_list_.getReadLen()), [this](const boost::system::error_code& err, size_t bytes_transferred){
			if (!err) {
				int res = send_buff_list_.readLen(bytes_transferred);
				if (res > 0) {
					// confirm send msg count
				}
			} else {
				sock_.close();
				session_mgr_->freeSessionById(getId());
				LibJmyLogError("async_write_some error: ", err.value());
				return;
			}
			sending_ = false;
		});
	} else {
		sock_.async_write_some(boost::asio::buffer(send_buff_.getReadBuff(), send_buff_.getReadLen()), [this](const boost::system::error_code& err, size_t bytes_transferred){
			if (!err) {
				send_buff_.readLen(bytes_transferred);
			} else {
				sock_.close();
				session_mgr_->freeSessionById(getId());
				LibJmyLogError("async_send error: ", err.value());
				return;
			}
			sending_ = false;
		});
	}
	sending_ = true;
	return 1;
}

int JmyTcpSession::run()
{
	int res = handle_send();
	return res;
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

JmyTcpSession* JmyTcpSessionMgr::getOneSession(std::shared_ptr<JmySessionBufferPool> pool,
												std::shared_ptr<JmyDataHandler> handler,
												bool use_send_list)
{
	if (free_session_list_.size() == 0)
		return NULL;

	JmyTcpSession* session = free_session_list_.front();
	if (!session)
		return NULL;

	curr_id_ += 1;
	if (curr_id_ > max_session_size_*2)
		curr_id_ = 1;

	if (!session->init(curr_id_, shared_from_this(), pool, handler, use_send_list))
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
				LibJmyLogError("session: %d run failed", s.second->getId()); 
			}
		}
	}
	return 0;
}
#endif
