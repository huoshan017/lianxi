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

	CsvParser::line_type& lines = parser.getLines();
	if (lines.size() < MIN_LINE_COUNT)
		return false;

	int i = 0;
	CsvParser::line_type::iterator it = lines.begin();
	for (; it!=lines.end(); ++it) {
		std::vector<std::string>& string_vec = *it;
		std::vector<std::string>::iterator vit = string_vec.begin();
		for (; vit!=string_vec.end(); ++vit) {
			if (i == NAME_LINE_INDEX) {
			} else if (i == TYPE_LINE_INDEX) {
			}
		}
		i += 1;
	}
	return true;
}

void TableLoader::clear()
{
}

