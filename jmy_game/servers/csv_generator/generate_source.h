#pragma once

#include <vector>
#include <string>
#include <fstream>
#include "csv_parser.h"

#define KEY_INDEX 0

class GenerateSource
{
public:
	GenerateSource();
	~GenerateSource();

	bool init(const char* table_name);
	void close();
	bool writeStruct(const std::vector<std::string>& name_line, const std::vector<std::string>& type_line);
	bool writeClassBody(const CsvParser::lines_type& lines);

private:
	bool writeLoadFunc();
	bool writeParseDataFunc(const CsvParser::lines_type& lines);
	bool writeCloseFunc(const CsvParser::lines_type& lines);
	bool writeGetFunc(const std::vector<std::string>& type_line);

	std::fstream out_file_;
	std::string file_name_;
	int key_index_;
	CsvParser parser_;
};
