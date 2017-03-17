#include "jmy_connection_buffer.h"
#include "jmy_mem.h"

JmyConnectionBufferMgr::JmyConnectionBufferMgr() : max_size_(0), conn_buff_vec_(nullptr)
{
}

JmyConnectionBufferMgr::~JmyConnectionBufferMgr()
{
}

bool JmyConnectionBufferMgr::init(int max_size)
{
	free_queue_.reverse(max_size);
	conn_buff_vec_ = (std::shared_ptr<JmyConnectionBuffer>*)jmy_mem_malloc(sizeof(std::shared_ptr<JmyConnectionBuffer>)*max_size);
	for (int i=0; i<max_size; ++i) {
		conn_buff_vec_[i] = std::make_shared<JmyConnectionBuffer>();
		conn_buff_vec_[i]->id = i+1;
		conn_buff_vec_[i]->is_used = false;
		free_queue_.push(conn_buff_vec_[i]);
	}
	max_size_ = max_size;
	return true;
}

void JmyConnectionBufferMgr::clear()
{
	free_queue_.clear();
	int i = 0;
	for (; i<max_size_; ++i) {
	}
}

bool JmyConnectionBufferMgr::getOneBuffer(std::shared_ptr<JmyConnectionBuffer>& buffer)
{
	return free_queue_.pop(buffer);
}

bool JmyConnectionBufferMgr::getBuffer(int id, std::shared_ptr<JmyConnectionBuffer>& buffer)
{
	if (id >= max_size_-1) return false;
	buffer = conn_buff_vec_[id];
	return true;
}

bool JmyConnectionBufferMgr::freeBuffer(int id)
{
	return true;
}

bool JmyConnectionBufferMgr::restoreBuffer(std::shared_ptr<JmyConnectionBuffer> buffer)
{
	return free_queue_.push(buffer);
}
