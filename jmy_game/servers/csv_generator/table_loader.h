#pragma once

#include "csv_parser.h"

class TableLoader
{
public:
	TableLoader();
	~TableLoader();

	bool load(const char* file_path);
	void clear();

private:
};
