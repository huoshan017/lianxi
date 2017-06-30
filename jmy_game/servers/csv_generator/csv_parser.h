#pragma once

#include <vector>
#include <string>

#define NAME_LINE_INDEX 0
#define TYPE_LINE_INDEX 1
#define DATA_LINE_START_INDEX 2

#define MIN_LINE_COUNT 3

class CsvParser
{
public :
	CsvParser();
	~CsvParser();

	bool load(const char* file_path);
	void close();

	typedef std::vector<std::vector<std::string> > lines_type;
	lines_type& getLines() { return lines_; }

private:
	void parseLine(const std::string& line, std::vector<std::string>& item_vec);
	lines_type lines_;
};
