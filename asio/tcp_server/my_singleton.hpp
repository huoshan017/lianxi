#pragma once

#include <cstddef>

template<typename T>
class MySingleton
{
public:
	~MySingleton() {
		destroyInstance();
	}
	static T* getInstance() {
		if (!instance_) {
			instance_ = new T;
		}
		return instance_;
	}
	void destroyInstance() {
		if (instance_) {
			delete instance_;
			instance_ = NULL;
		}
	}

protected:
	MySingleton() {}
	MySingleton(const MySingleton&);
	MySingleton& operator=(const MySingleton&);

protected:
	static T* instance_;
};

template<typename T>
T* MySingleton<T>::instance_ = NULL;
