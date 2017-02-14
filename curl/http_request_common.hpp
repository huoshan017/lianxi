#pragma once

#include <boost/lockfree/queue.hpp>
#include <boost/pool/object_pool.hpp>

template <typename T>
class ThreadSafeObjPool
{
public:
	ThreadSafeObjPool() : max_size_(0), free_queue_(0), use_pool_(false) {}
	~ThreadSafeObjPool() { clear(); }

	bool init(int max_size, bool use_pool = true) {
		if (max_size <= 0)
			return false;

		free_queue_.reserve(max_size);
		T* p = NULL;
		for (int i = 0; i<max_size; i++) {
			if (use_pool) {
				p = obj_pool_.malloc();
			} else {
				p = new T;
			}
			free_queue_.push(p);
			p->storeInMap();
		}

		max_size_ = max_size;
		use_pool_ = use_pool;
		return true;
	}

	void clear() {
		T* p = NULL;
		while (true) {
			if (free_queue_.empty())
				break;
			if (!free_queue_.pop(p))
				break;
			if (use_pool_) {
				obj_pool_.destroy(p);
			} else {
				delete p;
			}
			p->removeFromMap();
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
	bool use_pool_;
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
