#pragma once

#include <vector>
#include <string>
#include <fstream>

#define KEY_INDEX 0

class GenerateSource
{
public:
	GenerateSource();
	~GenerateSource();

	bool init(const char* table_name);
	void clear();
	bool writeStruct(std::vector<std::string>& fields_name, std::vector<std::string>& fields_type);
	bool writeClassBody();

private:
	bool writeLoadFunc();
	bool writeCloseFunc();

	std::fstream out_file_;
	std::string file_name_;
	int key_index_;
};
