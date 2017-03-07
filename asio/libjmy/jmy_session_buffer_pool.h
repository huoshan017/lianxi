#pragma once

#include "jmy_singleton.hpp"
#include <list>
#include <unordered_set>

const unsigned int DEFAULT_SEND_BUFFER_SIZE = 4096;
const unsigned int MAX_SEND_BUFFER_SIZE = 64*1024;
const unsigned int DEFAULT_RECV_BUFFER_SIZE = 4096;
const unsigned int MAX_RECV_BUFFER_SIZE = 64*1024;
const unsigned int DEFAULT_MAX_SEND_BUFFER_COUNT = 100;
const unsigned int DEFAULT_MAX_RECV_BUFFER_COUNT = 100;

class FixedSizeBufferPool
{
public:
	FixedSizeBufferPool() {}
	~FixedSizeBufferPool() { clear(); }
	bool init(int max_length, unsigned int fixed_size) {
		int i = 0;
		for (; i<max_length; i++) {
			free_buffer_list_.push_back(new char[fixed_size]);
		}
		max_length_ = max_length;
		fixed_size_ = fixed_size;
		return true;
	}
	void clear() {
		std::list<char*>::iterator it = free_buffer_list_.begin();
		for (; it!=free_buffer_list_.end(); ++it) {
			char* p = *it;
			if (p) { delete [] p; }
		}
		free_buffer_list_.clear();
		std::unordered_set<char*>::iterator sit = used_buffer_set_.begin();
		for (; sit!=used_buffer_set_.end(); ++sit) {
			char* p = *it;
			if (p) { delete [] p; }
		}
		used_buffer_set_.clear();
	}
	char* malloc() {
		if (free_buffer_list_.size() == 0)
			return NULL;
		char* p = free_buffer_list_.front();
		free_buffer_list_.pop_front();
		used_buffer_set_.insert(p);
		return p;
	}
	bool free(char* p) {
		std::unordered_set<char*>::iterator it = used_buffer_set_.find(p);
		if (it == used_buffer_set_.end())
			return false;
		free_buffer_list_.push_back(p);
		used_buffer_set_.erase(it);
		return true;
	}
	unsigned int getFixedSize() const { return fixed_size_; }

private:
	std::list<char*> free_buffer_list_;
	std::unordered_set<char*> used_buffer_set_;
	int max_length_;
	unsigned int fixed_size_;	
};

class JmySessionBufferPool
{
public:
	JmySessionBufferPool() : max_session_size_(0) {}
	~JmySessionBufferPool() { clear(); }
	bool init(int, unsigned int, unsigned int, unsigned int, unsigned int);
	void clear();
	char* mallocSendBuffer(unsigned int&);
	bool freeSendBuffer(char*);
	char* mallocRecvBuffer(unsigned int&);
	bool freeRecvBuffer(char*);
	char* mallocLargeSendBuffer(unsigned int&);
	bool freeLargeSendBuffer(char*);
	char* mallocLargeRecvBuffer(unsigned int&);
	bool freeLargeRecvBuffer(char*);

private:
	FixedSizeBufferPool send_buffer_pool_;
	FixedSizeBufferPool recv_buffer_pool_;
	FixedSizeBufferPool max_send_buffer_pool_;
	FixedSizeBufferPool max_recv_buffer_pool_;
	int max_session_size_;
};
