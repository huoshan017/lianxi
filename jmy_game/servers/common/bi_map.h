#pragma once

#include <unordered_map>

template <typename T1, typename T2>
class BiMap
{
public:
	BiMap() {}
	~BiMap() {}

	void clear() {
		one_map_.clear();
		two_map_.clear();
	}

	bool insert(const T1& t1, const T2& t2) {
		if (one_map_.find(t1) != one_map_.end())
			return false;
		if (two_map_.find(t2) != two_map_.end())
			return false;
		one_map_.insert(std::make_pair(t1, t2));
		two_map_.insert(std::make_pair(t2, t1));
		return true;
	}

	bool find_1(const T1& t1, T2& t2) {
		typename std::unordered_map<T1, T2>::iterator it = one_map_.find(t1);
		if (it == one_map_.end()) {
			return false;
		}
		t2 = it->second;
		return true;
	}

	bool find_2(const T2& t2, T1& t1) {
		typename std::unordered_map<T2, T1>::iterator it = two_map_.find(t2);
		if (it == two_map_.end()) {
			return false;
		}
		t1 = it->second;
		return true;
	}

	bool remove_1(const T1& t1) {
		typename std::unordered_map<T1, T2>::iterator it = one_map_.find(t1);
		if (it == one_map_.end())
			return false;
		one_map_.erase(t1);
		two_map_.erase(it->second);
		return true;
	}

	bool remove_2(const T2& t2) {
		typename std::unordered_map<T2, T1>::iterator it = two_map_.find(t2);
		if (it == two_map_.end())
			return false;
		two_map_.erase(t2);
		one_map_.erase(it->second);
		return true;
	}

	typedef std::unordered_map<T1, T2> one_map_type;
	typedef std::unordered_map<T2, T1> two_map_type;

	typename one_map_type::iterator one_map_begin() {
		return one_map_.begin();
	}

	typename one_map_type::iterator one_map_end() {
		return one_map_.end();
	}

	typename two_map_type::iterator two_map_begin() {
		return two_map_.begin();
	}

	typename two_map_type::iterator two_map_end() {
		return two_map_.end();
	}

private:
	std::unordered_map<T1, T2> one_map_;
	std::unordered_map<T2, T1> two_map_;
};
