#pragma once

#include <vector>
#include <string>
#include <fstream>
#include "../libjmy/jmy_csv_parser.h"

#define KEY_INDEX 0

class GenerateSource
{
public:
	GenerateSource();
	~GenerateSource();

	bool init(const char* table_name);
	void close();
	bool writeStruct(const std::vector<std::string>& name_line, const std::vector<std::string>& type_line);
	bool writeClassBody(const JmyCsvParser::lines_type& lines);

	const std::string& getFileName() const { return file_name_; }

private:
	bool writeLoadFunc();
	bool writeParseDataFunc(const JmyCsvParser::lines_type& lines);
	bool writeCloseFunc(const JmyCsvParser::lines_type& lines);
	bool writeGetFunc(const std::vector<std::string>& type_line);
	bool writeCountFunc();

	std::fstream out_file_;
	std::string file_name_;
	int key_index_;
	JmyCsvParser parser_;
};
