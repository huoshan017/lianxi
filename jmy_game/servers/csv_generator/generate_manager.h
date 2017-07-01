#pragma once

#include <fstream>
#include <list>
#include <string>

class GenerateManager
{
public:
	GenerateManager();
	~GenerateManager();
	void close();
	bool loadOne(const char* csv_name, const char* file_name, const char* class_name);
	bool generate();

private:
	struct CsvInfo {
		std::string file_name;
		std::string source_name;
		std::string class_name;
	};

	std::list<CsvInfo> csv_info_list_;;
	std::fstream out_file_;
};
