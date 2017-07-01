#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <cstdio>
#include <string.h>
#include "jmy_log.h"

#define NAME_LINE_INDEX 0
#define TYPE_LINE_INDEX 1
#define DATA_LINE_START_INDEX 2

#define MIN_LINE_COUNT 3

class JmyCsvParser
{
public :
	JmyCsvParser() {}
	~JmyCsvParser() {}

	bool load(const char* file_path) {
		std::ifstream infile(file_path);
		size_t item_num = 0;
		int n = 0;
		while (infile.good()) {
			std::string line;
			std::getline(infile, line);
			std::vector<std::string> item_vec;
			parseLine(line, item_vec);

			if (n == 0)
				item_num = item_vec.size();

			if (item_vec.size() > 0) {
				if (item_num != item_vec.size())  {
					close();
					//LibJmyLogError("file %s format invalid", file_path);
					return false;
				}
				lines_.push_back(item_vec);
			}
			n += 1;
		}
		infile.close();
		return true;
	}

	void close() {
		lines_.clear();
	}

	typedef std::vector<std::vector<std::string> > lines_type;
	lines_type& getLines() { return lines_; }

private:
	void parseLine(const std::string& line, std::vector<std::string>& item_vec) {
		char* p = (char*)line.c_str();
		char* s = strtok(p, ",\r");
		while (s != nullptr) {
			if (std::string(s) != "") {
				item_vec.push_back(s);
			}
			s = strtok(nullptr, ",\r");
		}
	}
	lines_type lines_;
};
