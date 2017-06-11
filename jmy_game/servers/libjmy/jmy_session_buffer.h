#pragma once

#include <list>
#include <memory>
#include <cstring>
#include <chrono>
#include <atomic>
#include "jmy_session_buffer_pool.h"
#include "jmy_datatype.h"
#include "jmy_net_proto.h"
#include "jmy_mem.h"
#include "jmy_log.h"

enum SessionBufferType {
	SESSION_BUFFER_TYPE_NONE,
	SESSION_BUFFER_TYPE_RECV,
	SESSION_BUFFER_TYPE_SEND,
};

class JmySimpleBuffer {
public:
	JmySimpleBuffer(): write_offset_(0) {
	}
	~JmySimpleBuffer() { }
	void reset() { write_offset_ = 0; }
	char* getReadBuff() { return buff_; }
	unsigned int getReadLen() { return write_offset_; }
	bool writeData(const char* data, unsigned int len) {
		if (write_offset_ > 0) {
			LibJmyLogError("not write data because of not reset");
			return false;
		}
		if (len > sizeof(buff_)) {
			LibJmyLogError("data len %d too long", len);
			return false;
		}
		std::memcpy(buff_, data, len);
		return true;
	}
	
private:
	char buff_[64];
	unsigned char write_offset_;
};

struct JmyData;

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
	bool writeData(JmyData* datas, int len);

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
	bool init(unsigned int size, SessionBufferType type);
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
	JmySessionBuffer& getSessionBuffer() { return use_large_?large_buff_:buff_; }

	bool writeLen(unsigned int len);
	bool readLen(unsigned int len);
	void moveDataToFront();
	bool writeData(const char* data, unsigned int len);
	bool writeData(JmyData* datas, int len);

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
	void destroy() {}

	bool init(unsigned int max_bytes = 0, unsigned int max_count = 0);
	void clear();

	bool writeData(const char* data, unsigned int len);
	bool writeData(JmyData* datas, int count);
	const char* getReadBuff();
	unsigned int getReadLen();
	int readLen(unsigned int len);
	void dropUsed(unsigned int len = 0);
	unsigned int getUsingSize() const { return using_list_.size(); }
	unsigned int getUsedSize() const { return used_list_.size(); }
	unsigned int getUsedBytes() const { return curr_used_bytes_; }
	JmyPacketType getCurrPacketType() {
		if (using_list_.size() == 0)
			return JMY_PACKET_NONE;
		buffer& b = using_list_.front();
		return (JmyPacketType)b.getPacketType();
	}

private:
	struct buffer {
		const char* data_;
		unsigned int len_;
		unsigned int roffset_;
		unsigned int woffset_;
		static std::atomic<uint64_t> init_count_;
		static std::atomic<uint64_t> uninit_count_;
		buffer() : data_(NULL), len_(0), roffset_(0), woffset_(0) {}
		buffer(buffer&& b) : data_(b.data_), len_(b.len_), roffset_(b.roffset_), woffset_(b.woffset_) {
			b.data_ = NULL;
		}
		buffer& operator=(buffer&& b) {
			data_ = b.data_;
			len_ = b.len_;
			roffset_ = b.roffset_;
			woffset_ = b.woffset_;
			b.data_ = NULL;
			return *this;
		}
		~buffer() { destroy(); }
		JmyPacketType getPacketType() const { return jmy_net_proto_pack_type(data_); }
		bool init(const char* data, unsigned int len) {
			if (!data || !len) return false;
			if (data_ && len_!=len) {
				jmy_mem_free((void*)data_); //delete [] data_;
				uninit_count_ += 1;
			}
			data_ = (const char*)jmy_mem_malloc(len);//new char[len];
			init_count_ += 1;
			std::memcpy((void*)data_, (void*)data, len);
			len_ = len;
			roffset_ = 0;
			woffset_ = len;
			return true;
		}
		bool init(unsigned int len) {
			if (!len) return false;
			if (data_ && len_!=len) {
				jmy_mem_free((void*)data_); //delete [] data_;
				uninit_count_ += 1;
			}
			data_ = (const char*)jmy_mem_malloc(len); //data_ = new char[len];
			init_count_ += 1;
			len_ = len;
			roffset_ = 0;
			woffset_ = 0;
			return true;
		}
		bool write(const char* data, unsigned int len) {
			if (!data || !len) return false;
			int left = len_ - woffset_;
			if (left < (int)len)
				return false;

			std::memcpy((void*)(data_+woffset_), (void*)data, len);
			woffset_ += len;
			return true;
		}
		const char* read_buff() {
			return data_ + roffset_;
		}
		unsigned int read_len() {
			if (woffset_ < roffset_) return 0;
			return woffset_ - roffset_;
		}
		bool read(unsigned int len) {
			if (len > woffset_-roffset_) return false;
			roffset_ += len;
			return true;
		}
		bool is_read_out() {
			if (roffset_ < len_) return false;
			return true;
		}
		void destroy() {
			if (data_) {
				jmy_mem_free((void*)data_); 
				uninit_count_ += 1;
				data_ = NULL;
			}
			len_ = 0;
			roffset_ = woffset_ = 0;
			//output_malloc_and_free_count();
		}
		void output_malloc_and_free_count() {
			static std::chrono::system_clock::time_point last_tick = std::chrono::system_clock::now();
			std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
			if (std::chrono::duration_cast<std::chrono::milliseconds>(now - last_tick).count() >= 1000) {
				last_tick = now;
				LibJmyLogInfo("init count(%llu)  uninit count(%llu)", init_count_.operator unsigned long(), uninit_count_.operator unsigned long());
			}
		}
	};

	std::list<buffer> using_list_;
	std::list<buffer> used_list_;
	unsigned int max_bytes_;
	unsigned int curr_used_bytes_;
	unsigned int max_count_;
	unsigned int curr_count_;
};

