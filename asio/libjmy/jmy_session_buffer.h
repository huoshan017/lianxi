#pragma once

#include <list>
#include <memory>
#include <cstring>
#include "jmy_session_buffer_pool.h"

enum SessionBufferType {
	SESSION_BUFFER_TYPE_NONE,
	SESSION_BUFFER_TYPE_RECV,
	SESSION_BUFFER_TYPE_SEND,
};

class JmySessionBuffer
{
public:
	JmySessionBuffer();
	JmySessionBuffer(unsigned int len);
	JmySessionBuffer(char* buff, unsigned int len);
	~JmySessionBuffer();

	bool init(unsigned int size, SessionBufferType type = SESSION_BUFFER_TYPE_NONE);
	bool init(char* buff, unsigned int size, SessionBufferType type = SESSION_BUFFER_TYPE_NONE);
	bool assign(const JmySessionBuffer& buffer);
	void destroy();
	void clear();

	bool isInited() const { return (buff_ && len_ > 0)?true:false; }
	char* getBuff() const { return buff_; }
	unsigned int getTotalLen() const;
	bool isEmpty() const;
	char* getWriteBuff();
	unsigned int getWriteLen() const;
	char* getReadBuff();
	unsigned int getReadLen() const;
	bool checkWriteLen(unsigned int len);
	SessionBufferType getType() const;

	bool writeLen(unsigned int len);
	bool readLen(unsigned int len);
	void moveDataToFront();
	bool writeData(const char* data, unsigned int len);

private:
	bool proxy_;
	char* buff_;
	unsigned int len_;
	SessionBufferType type_;
	unsigned int write_offset_;
	unsigned int read_offset_;
};

class JmyDoubleSessionBuffer
{
public:
	JmyDoubleSessionBuffer();
	~JmyDoubleSessionBuffer();
	bool init(std::shared_ptr<JmySessionBufferPool> pool, SessionBufferType type);
	void destroy();
	void clear();

	unsigned int getNormalLen() const;
	unsigned int getLargeLen() const;
	unsigned int getTotalLen() const { return use_large_?large_buff_.getTotalLen():buff_.getTotalLen(); }
	bool isLarge() const { return use_large_; }
	bool isEmpty() const;
	char* getWriteBuff();
	unsigned int getWriteLen() const;
	char* getReadBuff();
	unsigned int getReadLen() const;
	bool checkWriteLen(unsigned int len);
	char* getBuff() { return use_large_?large_buff_.getBuff():buff_.getBuff(); }

	bool writeLen(unsigned int len);
	bool readLen(unsigned int len);
	void moveDataToFront();
	bool writeData(const char* data, unsigned int len);

	bool switchToLarge();
	bool backToNormal();

private:
	bool use_large_;
	JmySessionBuffer buff_;
	JmySessionBuffer large_buff_;
	std::shared_ptr<JmySessionBufferPool> buff_pool_;
};

class JmySessionBufferList
{
public:
	JmySessionBufferList();
	~JmySessionBufferList();

	bool init(unsigned int max_bytes = 0, unsigned int max_count = 0);
	void destroy();

	bool writeData(const char* data, unsigned int len);
	const char* getReadBuff();
	unsigned int getReadLen();
	bool readLen(unsigned int len);

private:
	struct buffer {
		const char* data_;
		unsigned int len_;
		unsigned int roffset_;
		buffer() : data_(NULL), len_(0), roffset_(0) {}
		~buffer() { destroy(); }
		bool init(const char* data, unsigned int len) {
			if (!data || !len) return false;
			data_ = new char[len];
			std::memcpy((void*)data_, (void*)data, len);
			len_ = len;
			return true;
		}
		void destroy() {
			if (data_) {
				delete []data_;
				data_ = NULL;
				len_ = 0;
			}
		}
	};
	std::list<buffer> list_;
	std::list<buffer>::iterator curr_read_iter_;
	unsigned int max_bytes_;
	unsigned int curr_used_bytes_;
	unsigned int max_count_;
	unsigned int curr_count_;
};
