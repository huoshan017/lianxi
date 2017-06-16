#include "jmy_connection_buffer.h"
#include "jmy_mem.h"
#include "jmy_log.h"

JmyConnectionBufferMgr::JmyConnectionBufferMgr() : max_size_(0)
{
}

JmyConnectionBufferMgr::~JmyConnectionBufferMgr()
{
	clear();
}

bool JmyConnectionBufferMgr::init(int max_size)
{
	conn_buff_vec_.resize(max_size);
	for (int i=0; i<max_size; ++i) {
		conn_buff_vec_[i] = std::make_shared<JmyConnectionBuffer>();
		conn_buff_vec_[i]->id = i+1;
		conn_buff_vec_[i]->state = JMY_CONN_BUFFER_STATE_IDLE;
		free_queue_.push_back(conn_buff_vec_[i]);
	}
	max_size_ = max_size;
	return true;
}

bool JmyConnectionBufferMgr::init(int max_size, std::shared_ptr<JmySessionBufferPool> buff_pool)
{
	if (!init(max_size)) return false;
	buff_pool_ = buff_pool;
	return true;
}

void JmyConnectionBufferMgr::clear()
{
	free_queue_.clear();
}

bool JmyConnectionBufferMgr::getOneBuffer(std::shared_ptr<JmyConnectionBuffer>& buffer)
{
	if (free_queue_.size() == 0) return false;
	buffer = free_queue_.front();
	buffer->state = JMY_CONN_BUFFER_STATE_USING;
	free_queue_.pop_front();
	return true;
}

bool JmyConnectionBufferMgr::getBuffer(int id, std::shared_ptr<JmyConnectionBuffer>& buffer)
{
	if (id >= max_size_-1) return false;
	buffer = conn_buff_vec_[id];
	return true;
}

bool JmyConnectionBufferMgr::freeBuffer(int id)
{
	if (id<1 || id>max_size_) return false;
	if (conn_buff_vec_[id-1]->state == JMY_CONN_BUFFER_STATE_IDLE)
		return false;
	free_queue_.push_front(conn_buff_vec_[id-1]);
	LibJmyLogInfo("free buffer id(%d)", id);
	return true;
}
