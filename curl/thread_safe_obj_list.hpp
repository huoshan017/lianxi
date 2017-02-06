#pragma once

#include <boost/lockfree/queue.hpp>

template <typename T>
class ThreadSafeObjList 
{
public:
	ThreadSafeObjList() : max_size_(0), queue_(0) {}
	~ThreadSafeObjList() {}

	void init(int max_size) {
		queue_.reserve(max_size);
		max_size_ = max_size;
	}
	bool push(T* p) {
		return queue_.push(p);	
	}
	bool pop(T*& p) {
		return queue_.pop(p);
	}

private:
	boost::lockfree::queue<T*> queue_;
	int max_size_;
};
