#include "jmy_session_buffer.h"
#include "jmy_session_buffer_pool.h"
#include <cassert>
#include <memory.h>
#include <iostream>

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
	buff_ = new char[size];
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
		delete [] buff_;
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

bool JmyDoubleSessionBuffer::init(std::shared_ptr<JmySessionBufferPool> pool, SessionBufferType type)
{
	char* p = NULL;
	unsigned int s = 0;
	if (type == SESSION_BUFFER_TYPE_RECV)
		p = pool->mallocRecvBuffer(s);
	else
		p = pool->mallocSendBuffer(s);
	if (!p) {
		std::cout << "JmyDoubleSessionBuffer::init  failed to init because cant malloc buffer(" << type << ")" << std::endl;
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
	buff_.clear();
	large_buff_.clear();
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
			std::cout << "JmyDoubleSessionBuffer::switchToLarge  failed to malloc large buffer" << std::endl;
			return false;
		}
		if (!large_buff_.init(p, size, buff_.getType())) {
			std::cout << "JmyDoubleSessionBuffer::switchToLarge  failed to init large buffer" << std::endl;
			return false;
		}
	}

	if (!large_buff_.assign(buff_)) {
		std::cout << "JmyDoubleSessionBuffer::switchToLarge  failed to assign data to new large buffer" << std::endl;
		return false;
	}

	use_large_ = true;

	return true;
}

bool JmyDoubleSessionBuffer::backToNormal()
{
	if (!use_large_) return true;
	if (!buff_.assign(large_buff_)) {
		std::cout << "JmyDoubleSessionBuffer::backToNormal  failed to assign data old buffer" << std::endl;
		return false;
	}
	if (large_buff_.getType() == SESSION_BUFFER_TYPE_SEND) {
		if (!buff_pool_->freeLargeSendBuffer(large_buff_.getBuff())) {
			std::cout << "JmyDoubleSessionBuffer::backToNormal  failed to free large send buffer" << std::endl;
		}
	} else {
		if (!buff_pool_->freeLargeRecvBuffer(large_buff_.getBuff())) {
			std::cout << "JmyDoubleSessionBuffer::backToNormal  failed to free large recv buffer" << std::endl;		
		}
	}
	large_buff_.destroy();
	use_large_ = false;
	return true;
}
