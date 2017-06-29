#include "csv_parser.h"
#include <fstream>
#include "../libjmy/jmy_log.h"

CsvParser::CsvParser()
{
}

CsvParser::~CsvParser()
{
}

bool CsvParser::load(const char* file_path)
{
	std::ifstream infile(file_path);
	int n = 0;
	while (infile.good()) {
		std::string line;
		std::getline(infile, line);
		if (!parseLine(line)) {
			close();
			LibJmyLogError("parse line(%d) failed", n);
			return false;
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

bool CsvParser::parseLine(const std::string& line)
{
	size_t pos = line.find('\t');
	if (pos == 0 || pos==std::string::npos)
		return false;

	std::vector<std::string> item_vec;
	while (pos != std::string::npos) {
		item_vec.push_back(line.substr(0, pos));
		if (pos == 0)
			break;
		pos = line.find('\t', pos+1);
	}

	if (item_vec.size() == 0)
		return false;

	lines_.push_back(item_vec);
	return true;
}
