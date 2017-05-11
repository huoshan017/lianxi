#include "db_config_parser.h"
#include <iostream>

#define DB_STRUCT_JSON "./db_tables.json"

int main(int argc, char** argv)
{
	(void)argc;
	(void)argv;

	DBConfigParser p;
	if (!p.load(DB_STRUCT_JSON)) {
		std::cout << "load db struct json " << DB_STRUCT_JSON << " failed" << std::endl;
		return -1;
	}

	if (!p.generate()) {
		std::cout << "generate db struct source files failed" << std::endl;
		return -1;
	}

	p.clear();
	return 0;
}
