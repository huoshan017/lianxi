#pragma once

#include <cstddef>

template<typename T>
class JmySingleton
{
public:
	~JmySingleton() {
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
	JmySingleton() {}
	JmySingleton(const JmySingleton&);
	JmySingleton& operator=(const JmySingleton&);

protected:
	static T* instance_;
};

template<typename T>
T* JmySingleton<T>::instance_ = NULL;
