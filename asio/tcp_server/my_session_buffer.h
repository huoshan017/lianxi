#pragma once

class MySessionBuffer
{
public:
	MySessionBuffer();
	MySessionBuffer(unsigned int len);
	MySessionBuffer(char* buff, unsigned int len);
	~MySessionBuffer();

	bool init(unsigned int size);
	bool init(char* buff, unsigned int size);
	void destroy();
	void clear();

	unsigned int getTotalLen() const;
	bool isEmpty() const;
	char* getWriteBuff();
	unsigned int getWriteLen() const;
	char* getReadBuff();
	unsigned int getReadLen() const;
	bool checkWriteLen(unsigned int len);

	bool writeLen(unsigned int len);
	bool readLen(unsigned int len);
	void moveDataToFront();
	bool writeData(const char* data, unsigned int len);

private:
	bool proxy_;
	char* buff_;
	unsigned int len_;
	unsigned int write_offset_;
	unsigned read_offset_;
};
