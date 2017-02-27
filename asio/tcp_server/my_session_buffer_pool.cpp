#include "my_session_buffer_pool.h"

bool MySessionBufferPool::init(int max_session_size)
{
	if (!send_buffer_pool_.init(max_session_size)) return false;
	if (!recv_buffer_pool_.init(max_session_size)) return false;
	if (!max_send_buffer_pool_.init(DEFAULT_MAX_SEND_BUFFER_COUNT)) return false;
	if (!max_recv_buffer_pool_.init(DEFAULT_MAX_RECV_BUFFER_COUNT)) return false;
	max_session_size_ = max_session_size;
	return true;
}

void MySessionBufferPool::clear()
{
	send_buffer_pool_.clear();
	recv_buffer_pool_.clear();
	max_send_buffer_pool_.clear();
	max_recv_buffer_pool_.clear();
}

char* MySessionBufferPool::mallocSendBuffer(unsigned int& buffer_size)
{
	char* p = send_buffer_pool_.malloc();
	if (p) buffer_size = send_buffer_pool_.getFixedSize(); 
	return p;
}

bool MySessionBufferPool::freeSendBuffer(char* p)
{
	return send_buffer_pool_.free(p);
}

char* MySessionBufferPool::mallocRecvBuffer(unsigned int& buffer_size)
{
	char* p = recv_buffer_pool_.malloc();
	if (p) buffer_size = recv_buffer_pool_.getFixedSize();
	return p;
}

bool MySessionBufferPool::freeRecvBuffer(char* p)
{
	return recv_buffer_pool_.free(p);
}

char* MySessionBufferPool::mallocLargeSendBuffer(unsigned int& buffer_size)
{
	char* p = max_send_buffer_pool_.malloc();
	if (p) buffer_size = max_recv_buffer_pool_.getFixedSize();
	return p;
}
	

bool MySessionBufferPool::freeLargeSendBuffer(char* p)
{
	return max_send_buffer_pool_.free(p);
}

char* MySessionBufferPool::mallocLargeRecvBuffer(unsigned int& buffer_size)
{
	char* p = max_recv_buffer_pool_.malloc();
	if (p) buffer_size = max_send_buffer_pool_.getFixedSize();
	return p;
}
	

bool MySessionBufferPool::freeLargeRecvBuffer(char* p)
{
	return max_recv_buffer_pool_.free(p);
}
