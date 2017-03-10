#pragma once

#include <list>
#include <memory>
#include <cstring>
#include "jmy_session_buffer_pool.h"
#include "jmy_datatype.h"

enum SessionBufferType {
	SESSION_BUFFER_TYPE_NONE,
	SESSION_BUFFER_TYPE_RECV,
	SESSION_BUFFER_TYPE_SEND,
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
	bool writeData(JmyData* datas, int len);

	bool switchToLarge();
	bool backToNormal();

private:
	bool use_large_;
	JmySessionBuffer buff_;
	JmySessionBuffer large_buff_;
	std::shared_ptr<JmySessionBufferPool> buff_pool_;
};

struct JmyBufferDropConditionData {
	uint32_t conds;
	uint32_t params[DropConditionCount];
	JmyBufferDropConditionData() {
		conds &= DropConditionImmidate;
		std::memset(params, 0, sizeof(params));
	}
	bool hasCond(JmyBufferDropCondition cond) {
		return conds & cond;
	}
	uint32_t getParam(JmyBufferDropCondition cond) {
		return params[cond];
	}
	void setCond(JmyBufferDropCondition cond, uint32_t param) {
		conds &= cond;
		params[cond] = param;
	}
};

class JmySessionBufferList
{
public:
	JmySessionBufferList();
	~JmySessionBufferList();

	bool init(unsigned int max_bytes = 0, unsigned int max_count = 0);
	void reset();
	void addDropCondition(JmyBufferDropCondition cond, uint32_t param);

	bool writeData(const char* data, unsigned int len);
	bool writeData(JmyData* datas, int count);
	const char* getReadBuff();
	unsigned int getReadLen();
	bool readLen(unsigned int len);
	void dropUsed(unsigned int len = 0);
	unsigned int getUsingSize() const { return using_list_.size(); }
	unsigned int getUsedSize() const { return used_list_.size(); }

private:
	struct buffer {
		const char* data_;
		unsigned int len_;
		unsigned int roffset_;
		unsigned int woffset_;
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
		bool init(const char* data, unsigned int len) {
			if (!data || !len) return false;
			if (data_ && len_!=len) {
				delete [] data_;
			}
			data_ = new char[len];
			std::memcpy((void*)data_, (void*)data, len);
			len_ = len;
			roffset_ = 0;
			woffset_ = len;
			return true;
		}
		bool init(unsigned int len) {
			if (!len) return false;
			if (data_ && len_!=len) {
				delete [] data_;
			}
			data_ = new char[len];
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
				delete []data_;
				data_ = NULL;
			}
			len_ = 0;
			roffset_ = woffset_ = 0;
		}
	};
	std::list<buffer> using_list_;
	std::list<buffer> used_list_;
	unsigned int max_bytes_;
	unsigned int curr_used_bytes_;
	unsigned int max_count_;
	unsigned int curr_count_;
	JmyBufferDropConditionData drop_cond_;
};
