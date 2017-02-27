#include "my_session_buffer.h"
#include <cassert>
#include <memory.h>

MySessionBuffer::MySessionBuffer() : proxy_(false), buff_(NULL), len_(0)
{
}

MySessionBuffer::MySessionBuffer(unsigned int len) : proxy_(false),  buff_(NULL), len_(len) 
{
}

MySessionBuffer::MySessionBuffer(char* buff, unsigned int len) : proxy_(true), buff_(buff), len_(len)
{
}

MySessionBuffer::~MySessionBuffer()
{
	destroy();
}

bool MySessionBuffer::init(unsigned int size)
{
	destroy();
	buff_ = new char[size];
	len_ = size;
	proxy_ = false;
	return true;
}

bool MySessionBuffer::init(char* buff, unsigned int size)
{
	destroy();
	buff_ = buff;
	len_ = size;
	proxy_ = true;
	return true;
}

void MySessionBuffer::destroy()
{
	if (!proxy_ && buff_) {
		delete [] buff_;
	}
	buff_ = NULL;
	len_ = 0;
	write_offset_ = read_offset_ = 0;
}

void MySessionBuffer::clear()
{
	write_offset_ = read_offset_ = 0;
}

unsigned int MySessionBuffer::getTotalLen() const
{
	return len_;
}

bool MySessionBuffer::isEmpty() const
{
	return (write_offset_ == read_offset_);
}
	
char* MySessionBuffer::getWriteBuff()
{
	return buff_ + write_offset_;
}

unsigned int MySessionBuffer::getWriteLen() const
{
	return len_ - write_offset_;
}

char* MySessionBuffer::getReadBuff()
{
	return buff_ + read_offset_;
}

unsigned int MySessionBuffer::getReadLen() const
{
	return write_offset_ - read_offset_;
}

bool MySessionBuffer::checkWriteLen(unsigned int len)
{
	unsigned int left_write = getWriteLen();
	if (left_write < len) {
		moveDataToFront();
	}
	left_write = getWriteLen();
	return left_write >= len;
}

bool MySessionBuffer::writeLen(unsigned int len)
{
	unsigned int left_write = getWriteLen();
	if (left_write < len) return false;
	write_offset_ += len;
	return true;
}

bool MySessionBuffer::readLen(unsigned int len)
{
	unsigned int left_read = getReadLen();
	if (left_read < len) return false;
	read_offset_ += len;
	return true;
}

void MySessionBuffer::moveDataToFront()
{
	unsigned int left_read = getReadLen();
	memmove(buff_, buff_+read_offset_, left_read);
	read_offset_ = 0;
	write_offset_ = left_read;
}

bool MySessionBuffer::writeData(const char* data, unsigned int len)
{
	unsigned int left_write = getWriteLen();
	if (left_write < len) {
		return false;
	}
	memcpy(buff_+write_offset_, data, len);
	return true;
}
