#pragma once

#include <unordered_map>
#include <vector>

template <typename T, typename KeyType>
class JmyObjectSet
{
public:
	JmyObjectSet() {}
	~JmyObjectSet() {}

	size_t count() const {
		return objs_vec_.size();
	}

	void clear(bool free_memory = false) {
		if (free_memory) {
			typename std::vector<T*>::iterator it = objs_vec_.begin();
			for (; it!=objs_vec_.end(); ++it) {
				delete *it;
			}
		}
		objs_map_.clear();
		objs_vec_.clear();
	}

	void insertKeyValue(const KeyType& key, T* value) {
		objs_map_.insert(std::make_pair(key, value));
	}

	bool removeValueByKey(const KeyType& key) {
		if (objs_map_.find(key) == objs_map_.end())
			return false;
		objs_map_.erase(key);
		return true;
	}

	void pushValue(T* value) {
		objs_vec_.push_back(value);
	}

	bool removeValue(T* value) {
		typename std::vector<T*>::iterator it = objs_vec_.begin();
		for (; it!=objs_vec_.end(); ++it) {
			if (value == *it) {
				objs_vec_.erase(it);
				return true;
			}
		}
		return false;
	}

	bool removeValueByIndex(uint32_t index) {
		return true;
	}

	T* getByKey(const KeyType& key) {
		typename std::unordered_map<KeyType, T*>::iterator it = objs_map_.find(key);
		if (it == objs_map_.end())
			return nullptr;
		return it->second;
	}

	T* getByIndex(uint32_t index) {
		if (index >= objs_vec_.size())
			return nullptr;
		return objs_vec_[index];
	}

private:
	std::unordered_map<KeyType, T*> objs_map_;
	std::vector<T*> objs_vec_;
};
