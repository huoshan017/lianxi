#include "csv_parser.h"
#include <iostream>
#include <fstream>
#include <cstdio>
#include <string.h>

CsvParser::CsvParser()
{
}

CsvParser::~CsvParser()
{
}

bool CsvParser::load(const char* file_path)
{
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
				std::cout << "file " << file_path << " format invalid" << std::endl;
				return false;
			}
			lines_.push_back(item_vec);
		}
		n += 1;
	}
	infile.close();
	return true;
}

void CsvParser::close()
{
	lines_.clear();
}

void CsvParser::parseLine(const std::string& line, std::vector<std::string>& item_vec)
{
	char* p = (char*)line.c_str();
	char* s = strtok(p, ",\r");
	while (s != nullptr) {
		std::cout << "ch: ";
		for (int i=0; i<(int)strlen(s); ++i) {
			std::cout << (int)s[i] << " ";
		}
		std::cout << std::endl;

		item_vec.push_back(s);
		s = strtok(nullptr, ",");
	}
}
