#include "jmy_session_buffer.h"
#include "jmy_session_buffer_pool.h"
#include "jmy_datatype.h"
#include "jmy_mem.h"
#include "jmy_log.h"
#include <cassert>
#include <memory.h>
#include <cassert>

JmySessionBuffer::JmySessionBuffer()
	: proxy_(false), buff_(NULL), len_(0), type_(SESSION_BUFFER_TYPE_NONE), write_offset_(0), read_offset_(0)
{
}

JmySessionBuffer::JmySessionBuffer(unsigned int len)
	: proxy_(false),  buff_(NULL), len_(len), type_(SESSION_BUFFER_TYPE_NONE), write_offset_(0), read_offset_(0)
{
}

JmySessionBuffer::JmySessionBuffer(char* buff, unsigned int len)
	: proxy_(true), buff_(buff), len_(len), type_(SESSION_BUFFER_TYPE_NONE), write_offset_(0), read_offset_(0)
{
}

JmySessionBuffer::~JmySessionBuffer()
{
	destroy();
}

bool JmySessionBuffer::init(unsigned int size, SessionBufferType type)
{
	destroy();
	buff_ = (char*)jmy_mem_malloc(size);
	len_ = size;
	type_ = type;
	proxy_ = false;
	return true;
}

bool JmySessionBuffer::init(char* buff, unsigned int size, SessionBufferType type)
{
	destroy();
	buff_ = buff;
	len_ = size;
	type_ = type;
	proxy_ = true;
	return true;
}

void JmySessionBuffer::destroy()
{
	if (!proxy_ && buff_) {
		jmy_mem_free(buff_);
	}
	buff_ = NULL;
	len_ = 0;
	write_offset_ = read_offset_ = 0;
}

void JmySessionBuffer::clear()
{
	write_offset_ = read_offset_ = 0;
}

bool JmySessionBuffer::assign(const JmySessionBuffer& buffer)
{
	write_offset_ = buffer.write_offset_ - buffer.read_offset_;
	read_offset_ = 0;
	if (write_offset_ > 0) {
		memcpy(buff_, buffer.buff_+buffer.write_offset_, write_offset_);
	}
	return true;
}

unsigned int JmySessionBuffer::getTotalLen() const
{
	return len_;
}

bool JmySessionBuffer::isEmpty() const
{
	return (write_offset_ == read_offset_);
}
	
char* JmySessionBuffer::getWriteBuff()
{
	return buff_ + write_offset_;
}

unsigned int JmySessionBuffer::getWriteLen() const
{
	return len_ - write_offset_;
}

char* JmySessionBuffer::getReadBuff()
{
	return buff_ + read_offset_;
}

unsigned int JmySessionBuffer::getReadLen() const
{
	return write_offset_ - read_offset_;
}

bool JmySessionBuffer::checkWriteLen(unsigned int len)
{
	unsigned int left_write = getWriteLen();
	if (left_write < len) {
		moveDataToFront();
		left_write = getWriteLen();
	}
	return left_write >= len;
}

SessionBufferType JmySessionBuffer::getType() const
{
	return type_;
}

bool JmySessionBuffer::writeLen(unsigned int len)
{
	unsigned int left_write = getWriteLen();
	if (left_write < len) return false;
	write_offset_ += len;
	return true;
}

bool JmySessionBuffer::readLen(unsigned int len)
{
	unsigned int left_read = getReadLen();
	if (left_read < len) return false;
	read_offset_ += len;
	return true;
}

void JmySessionBuffer::moveDataToFront()
{
	unsigned int left_read = getReadLen();
	memmove(buff_, buff_+read_offset_, left_read);
	read_offset_ = 0;
	write_offset_ = left_read;
}

bool JmySessionBuffer::writeData(const char* data, unsigned int len)
{
	unsigned int left_write = getWriteLen();
	if (left_write < len) {
		return false;
	}
	memcpy(buff_+write_offset_, data, len);
	write_offset_ += len;
	return true;
}

bool JmySessionBuffer::writeData(JmyData* datas, int len)
{
	if (!datas || !len)
		return false;

	unsigned int total_len = 0;
	int i = 0;
	for (; i<len; i++) {
		total_len += datas[i].len;
	}

	if (total_len > getWriteLen()) {
		return false;
	}

	for (i=0; i<len; i++) {
		memcpy(buff_+write_offset_, datas[i].data, datas[i].len);
		write_offset_ += datas[i].len;
	}

	return true;
}

/**
 * JmyDoubleSessionBuffer
 */
JmyDoubleSessionBuffer::JmyDoubleSessionBuffer() : use_large_(false)
{
}

JmyDoubleSessionBuffer::~JmyDoubleSessionBuffer()
{
	destroy();
}

bool JmyDoubleSessionBuffer::init(unsigned int size, SessionBufferType type)
{
	assert(size > 0);
	if (!buff_.init(size, type))
		return false;
	return true;
}

bool JmyDoubleSessionBuffer::init(std::shared_ptr<JmySessionBufferPool> pool, SessionBufferType type)
{
	char* p = NULL;
	unsigned int s = 0;
	if (type == SESSION_BUFFER_TYPE_RECV)
		p = pool->mallocRecvBuffer(s);
	else
		p = pool->mallocSendBuffer(s);
	if (!p) {
		LibJmyLogError("failed to init because cant malloc buffer(%d)", type);
		assert(0);
		return false;
	}
	assert(s > 0);
	if (!buff_.init(p, s, type))
		return false;

	buff_pool_ = pool;
	return true;
}

void JmyDoubleSessionBuffer::destroy()
{
	buff_.destroy();
	large_buff_.destroy();
}

void JmyDoubleSessionBuffer::clear()
{
	if (buff_pool_.get()) {
		if (buff_.getBuff()) {
			if (buff_.getType() == SESSION_BUFFER_TYPE_RECV) {
				buff_pool_->freeRecvBuffer(buff_.getBuff());
			} else {
				buff_pool_->freeSendBuffer(buff_.getBuff());
			}
		}
		if (large_buff_.getBuff()) {
			if (large_buff_.getType() == SESSION_BUFFER_TYPE_RECV) {
				buff_pool_->freeLargeRecvBuffer(large_buff_.getBuff());
			} else {
				buff_pool_->freeLargeSendBuffer(large_buff_.getBuff());
			}
		}
	} else {
		buff_.clear();
		large_buff_.clear();
	}
}

unsigned int JmyDoubleSessionBuffer::getNormalLen() const
{
	return buff_.getTotalLen();
}

unsigned int JmyDoubleSessionBuffer::getLargeLen() const
{
	return large_buff_.getTotalLen();
}

bool JmyDoubleSessionBuffer::isEmpty() const
{
	if (!use_large_) return buff_.isEmpty();
	return large_buff_.isEmpty();
}

char* JmyDoubleSessionBuffer::getWriteBuff()
{
	if (!use_large_) return buff_.getWriteBuff();
	return large_buff_.getWriteBuff();
}

unsigned int JmyDoubleSessionBuffer::getWriteLen() const
{
	if (!use_large_) return buff_.getWriteLen();
	return large_buff_.getWriteLen();
}
	
char* JmyDoubleSessionBuffer::getReadBuff()
{
	if (!use_large_) return buff_.getReadBuff();
	return large_buff_.getReadBuff();
}

unsigned int JmyDoubleSessionBuffer::getReadLen() const
{
	if (!use_large_) return buff_.getReadLen();
	return large_buff_.getReadLen();
}

bool JmyDoubleSessionBuffer::checkWriteLen(unsigned int len)
{
	if (!use_large_) return buff_.checkWriteLen(len);
	return large_buff_.checkWriteLen(len);
}

bool JmyDoubleSessionBuffer::writeLen(unsigned int len)
{
	if (!use_large_) return buff_.writeLen(len);
	return large_buff_.writeLen(len);
}

bool JmyDoubleSessionBuffer::readLen(unsigned int len)
{
	if (!use_large_) return buff_.readLen(len);
	return large_buff_.readLen(len);
}
	
void JmyDoubleSessionBuffer::moveDataToFront()
{
	if (!use_large_) buff_.moveDataToFront();
	return large_buff_.moveDataToFront();
}

bool JmyDoubleSessionBuffer::writeData(const char* data, unsigned int len)
{
	if (!use_large_) return buff_.writeData(data, len);
	return large_buff_.writeData(data, len);
}

bool JmyDoubleSessionBuffer::writeData(JmyData* datas, int len)
{
	if (!use_large_) return buff_.writeData(datas, len);
	return large_buff_.writeData(datas, len);
}

bool JmyDoubleSessionBuffer::switchToLarge()
{
	if (use_large_) return true;

	if (!large_buff_.isInited()) {
		char* p = NULL;
		unsigned int size = 0;
		if (buff_.getType() == SESSION_BUFFER_TYPE_SEND) 
			p = buff_pool_->mallocLargeSendBuffer(size);
		else
			p = buff_pool_->mallocLargeRecvBuffer(size);
		if (!p) {
			LibJmyLogError("failed to malloc large buffer");
			return false;
		}
		if (!large_buff_.init(p, size, buff_.getType())) {
			LibJmyLogError("failed to init large buffer");
			return false;
		}
	}

	if (!large_buff_.assign(buff_)) {
		LibJmyLogError("failed to assign data to new large buffer");
		return false;
	}

	use_large_ = true;

	return true;
}

bool JmyDoubleSessionBuffer::backToNormal()
{
	if (!use_large_) return true;
	if (!buff_.assign(large_buff_)) {
		LibJmyLogError("failed to assign data old buffer");
		return false;
	}
	if (large_buff_.getType() == SESSION_BUFFER_TYPE_SEND) {
		if (!buff_pool_->freeLargeSendBuffer(large_buff_.getBuff())) {
			LibJmyLogError("failed to free large send buffer");
		}
	} else {
		if (!buff_pool_->freeLargeRecvBuffer(large_buff_.getBuff())) {
			LibJmyLogError("failed to free large recv buffer");		
		}
	}
	large_buff_.destroy();
	use_large_ = false;
	return true;
}

/**
 * JmySessionBufferList
 */

JmySessionBufferList::JmySessionBufferList() :
	max_bytes_(0), curr_used_bytes_(0), max_count_(0), curr_count_(0)
{
}

JmySessionBufferList::~JmySessionBufferList()
{
}

bool JmySessionBufferList::init(unsigned int max_bytes, unsigned int max_count)
{
	max_bytes_ = max_bytes;
	max_count_ = max_count;
	return true;
}

void JmySessionBufferList::clear()
{
	using_list_.clear();
	curr_used_bytes_ = 0;
	curr_count_ = 0;
}

bool JmySessionBufferList::writeData(const char* data, unsigned short len)
{
	if (max_count_>0 && curr_count_+1>max_count_) {
		LibJmyLogError("buffer list used data count(%d) is max", max_count_);
		return false;
	}

	if (max_bytes_>0 && curr_used_bytes_+len>max_bytes_) {
		LibJmyLogError("buffer list used bytes(%d) great to max bytes(%d)", curr_used_bytes_+len, max_bytes_);
		return false;
	}
	
	buffer b;
	if (!b.init(data, len)) {
		LibJmyLogError("buffer init failed");
		return false;
	}

	using_list_.push_back(std::move(b));
	if (max_count_ > 0)
		curr_count_ += 1;
	if (max_bytes_ > 0)
		curr_used_bytes_ += len;

	return true;
}

bool JmySessionBufferList::writeData(JmyData* datas, int count)
{
	if (!datas || !count)
		return false;

	if (max_count_>0 && curr_count_+1>max_count_) {
		LibJmyLogError("buffer list used data count(%d) is max", max_count_);
		return false;
	}

	unsigned int total_len = 0;
	int i = 0;
	for (; i<count; ++i) {
		total_len += datas[i].len;
	}

	if (max_bytes_>0 && curr_used_bytes_+total_len>max_bytes_) {
		LibJmyLogError("buffer list used bytes(%d) great to max bytes(%d)", curr_used_bytes_+total_len, max_bytes_);
		return false;
	}

	buffer b;
	if (!b.init(total_len)) {
		LibJmyLogError("init new buffer by length(%d) failed", total_len);
		return false;
	}

	for (i=0; i<count; ++i) {
		if (!b.write(datas[i].data, datas[i].len))
			return false;
	}

	using_list_.push_back(std::move(b));
	if (max_count_ > 0)
		curr_count_ += 1;
	if (max_bytes_ > 0)
		curr_used_bytes_ += total_len;

	return true;
}

bool JmySessionBufferList::writeData(unsigned short param, const char* data, unsigned short len)
{
	buffer b;
	if (!b.init(data, len)) {
		LibJmyLogError("buffer init failed");
		return false;
	}

	b.woffset_ = param;
	using_list_.push_back(std::move(b));

	return true;
}

const char* JmySessionBufferList::getReadBuff()
{
	if (using_list_.size() == 0)
		return NULL;
	buffer& b = using_list_.front();
	return b.read_buff();
}

unsigned int JmySessionBufferList::getReadLen()
{
	if (using_list_.size() == 0)
		return 0;
	buffer& b = using_list_.front();
	unsigned int read_len = b.read_len();
	return read_len;
}

int JmySessionBufferList::readLen(unsigned int len)
{
	if (using_list_.size() == 0)
		return 0;

	buffer& b = using_list_.front();
	if (!b.read(len))
		return 0;

	if (b.is_read_out()) {
		using_list_.pop_front();
		//LibJmyLogInfo("after pop front, buff list size: %d", using_list_.size());
		return len;
	}
	return len;
}
