#include "jmy_session_buffer_pool.h"
#include "jmy_log.h"

bool JmySessionBufferPool::init(int max_session_size,
		unsigned int min_send_buff_size, unsigned int min_recv_buff_size,
		unsigned int max_send_buff_size, unsigned int max_recv_buff_size)
{
	if (min_send_buff_size <= DEFAULT_SEND_BUFFER_SIZE/2)
		min_send_buff_size = DEFAULT_SEND_BUFFER_SIZE/2;
	else if (min_send_buff_size <= DEFAULT_SEND_BUFFER_SIZE)
		min_send_buff_size = DEFAULT_SEND_BUFFER_SIZE;
	else {
		LibJmyLogError("min_send_buff_size(%d) > DEFAULT_SEND_BUFFER_SIZE(%d)", min_send_buff_size, DEFAULT_SEND_BUFFER_SIZE);
		return false;
	}

	if (min_recv_buff_size <= DEFAULT_RECV_BUFFER_SIZE/2) 
		min_recv_buff_size = DEFAULT_RECV_BUFFER_SIZE/2;
	else if (min_recv_buff_size <= DEFAULT_RECV_BUFFER_SIZE)
		min_recv_buff_size = DEFAULT_RECV_BUFFER_SIZE;
	else {
		LibJmyLogError("min_recv_buff_size(%d) > DEFAULT_RECV_BUFFER_SIZE(%d)", min_recv_buff_size, DEFAULT_RECV_BUFFER_SIZE);
		return false;
	}

	if (max_send_buff_size <= MAX_SEND_BUFFER_SIZE/2)
		max_send_buff_size = MAX_SEND_BUFFER_SIZE/2;
	else if (max_send_buff_size <= MAX_SEND_BUFFER_SIZE)
		max_send_buff_size = MAX_SEND_BUFFER_SIZE;
	else {
		LibJmyLogError("max_send_buff_size(%d) > MAX_SEND_BUFFER_SIZE(%d)", max_send_buff_size, MAX_SEND_BUFFER_SIZE);
		return false;
	}

	if (max_recv_buff_size <= MAX_RECV_BUFFER_SIZE/2)
		max_recv_buff_size = MAX_RECV_BUFFER_SIZE/2;
	else if (max_recv_buff_size <= MAX_RECV_BUFFER_SIZE)
		max_recv_buff_size = MAX_RECV_BUFFER_SIZE;
	else {
		LibJmyLogError("max_recv_buff_size(%d) > MAX_RECV_BUFFER_SIZE(%d)", max_recv_buff_size, MAX_RECV_BUFFER_SIZE);
		return false;
	}
		
	if (!send_buffer_pool_.init(max_session_size, min_send_buff_size)) return false;
	if (!recv_buffer_pool_.init(max_session_size, min_recv_buff_size)) return false;
	if (!max_send_buffer_pool_.init(DEFAULT_MAX_SEND_BUFFER_COUNT, max_send_buff_size)) return false;
	if (!max_recv_buffer_pool_.init(DEFAULT_MAX_RECV_BUFFER_COUNT, max_recv_buff_size)) return false;
	max_session_size_ = max_session_size;
	return true;
}

void JmySessionBufferPool::clear()
{
	send_buffer_pool_.clear();
	recv_buffer_pool_.clear();
	max_send_buffer_pool_.clear();
	max_recv_buffer_pool_.clear();
}

char* JmySessionBufferPool::mallocSendBuffer(unsigned int& buffer_size)
{
	char* p = send_buffer_pool_.malloc();
	if (p) buffer_size = send_buffer_pool_.getFixedSize(); 
	return p;
}

bool JmySessionBufferPool::freeSendBuffer(char* p)
{
	return send_buffer_pool_.free(p);
}

char* JmySessionBufferPool::mallocRecvBuffer(unsigned int& buffer_size)
{
	char* p = recv_buffer_pool_.malloc();
	if (p) buffer_size = recv_buffer_pool_.getFixedSize();
	return p;
}

bool JmySessionBufferPool::freeRecvBuffer(char* p)
{
	return recv_buffer_pool_.free(p);
}

char* JmySessionBufferPool::mallocLargeSendBuffer(unsigned int& buffer_size)
{
	char* p = max_send_buffer_pool_.malloc();
	if (p) buffer_size = max_recv_buffer_pool_.getFixedSize();
	return p;
}
	

bool JmySessionBufferPool::freeLargeSendBuffer(char* p)
{
	return max_send_buffer_pool_.free(p);
}

char* JmySessionBufferPool::mallocLargeRecvBuffer(unsigned int& buffer_size)
{
	char* p = max_recv_buffer_pool_.malloc();
	if (p) buffer_size = max_send_buffer_pool_.getFixedSize();
	return p;
}
	

bool JmySessionBufferPool::freeLargeRecvBuffer(char* p)
{
	return max_recv_buffer_pool_.free(p);
}
