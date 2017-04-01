#include "jmy_connection_buffer.h"
#include "jmy_mem.h"
#include "jmy_log.h"

JmyConnectionBufferMgr::JmyConnectionBufferMgr() : max_size_(0), conn_buff_vec_(nullptr)
{
}

JmyConnectionBufferMgr::~JmyConnectionBufferMgr()
{
	clear();
}

bool JmyConnectionBufferMgr::init(int max_size)
{
	conn_buff_vec_ = (std::shared_ptr<JmyConnectionBuffer>*)jmy_mem_malloc(sizeof(std::shared_ptr<JmyConnectionBuffer>)*max_size);
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
	suspend_map_.clear();
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
	if (id >= max_size_-1) return false;
	conn_buff_vec_[id].reset();
	return true;
}

void JmyConnectionBufferMgr::restoreBuffer(std::shared_ptr<JmyConnectionBuffer> buffer)
{
	buffer->state = JMY_CONN_BUFFER_STATE_IDLE;
	free_queue_.push_back(buffer);
}

bool JmyConnectionBufferMgr::suspendBuffer(std::shared_ptr<JmyConnectionBuffer> buffer)
{
	if (buffer->state == JMY_CONN_BUFFER_STATE_SUSPEND) {
		LibJmyLogWarn("connection buffer(%d) already state(%d), cant suspend", buffer->id, buffer->state);
		return false;
	}
	buffer->state = JMY_CONN_BUFFER_STATE_SUSPEND;
	suspend_map_.insert(make_pair(buffer->id, buffer));
	return true;
}

bool JmyConnectionBufferMgr::getSuspendBuffer(int id, std::shared_ptr<JmyConnectionBuffer>& buffer)
{
	std::unordered_map<int, std::shared_ptr<JmyConnectionBuffer> >::iterator it = suspend_map_.find(id);
	if (it == suspend_map_.end()) return false;
	buffer = it->second;
	return true;
}

bool JmyConnectionBufferMgr::restoreSuspendBuffer(int id)
{
	std::unordered_map<int, std::shared_ptr<JmyConnectionBuffer> >::iterator it = suspend_map_.find(id);
	if (it == suspend_map_.end()) return false;
	restoreBuffer(it->second);
	return true;
}
