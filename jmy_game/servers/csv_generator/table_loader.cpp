#include "table_loader.h"

TableLoader::TableLoader()
{
}

TableLoader::~TableLoader()
{
}

bool TableLoader::load(const char* file_path)
{
	CsvParser parser;
	if (!parser.load(file_path))
		return false;

	CsvParser::lines_type& lines = parser.getLines();
	if (lines.size() < MIN_LINE_COUNT)
		return false;

	return true;
}

void TableLoader::clear()
{
}

