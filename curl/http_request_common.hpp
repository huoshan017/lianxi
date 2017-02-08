#pragma once

#include <boost/lockfree/queue.hpp>
#include <boost/pool/object_pool.hpp>

template <typename T>
class ThreadSafeObjPool
{
public:
	ThreadSafeObjPool() : max_size_(0), free_queue_(0) {}
	~ThreadSafeObjPool() { clear(); }

	bool init(int max_size) {
		if (max_size <= 0)
			return false;

		free_queue_.reserve(max_size);
		T* p = NULL;
		for (int i = 0; i<max_size; i++) {
			p = obj_pool_.malloc();
			free_queue_.push(p);
		}

		max_size_ = max_size;
		return true;
	}

	void clear() {
		T* p = NULL;
		while (true) {
			if (free_queue_.empty())
				break;
			if (!free_queue_.pop(p))
				break;
			obj_pool_.destroy(p);
		}
	}

	T* malloc() {
		T* p = NULL;
		if (!free_queue_.pop(p))
			return NULL;

		return p;
	}

	bool free(T* p) {
		return free_queue_.push(p);
	}

	bool hasFree() {
		return !free_queue_.empty();
	}

private:
	int max_size_;
	boost::lockfree::queue<T*> free_queue_;
	boost::object_pool<T> obj_pool_;
};

template <typename T>
class ThreadSafeObjList 
{
public:
	ThreadSafeObjList() : queue_(0), max_size_(0) {}
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
