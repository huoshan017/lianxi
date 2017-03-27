#include "config_loader.h"
#include <fstream>
#include <string>
#include <cassert>
#include <iostream>
#include "util.h"

ConfigLoader::ConfigLoader()
{
}

ConfigLoader::~ConfigLoader()
{
	close();
}

bool ConfigLoader::loadJson(const char* jsonpath)
{
	std::ifstream in;

	in.open(jsonpath, std::ifstream::in);
	if (!in.is_open()) {
		std::cerr << "failed to open " << jsonpath << std::endl;
		return false;
	}

	std::string line;
	std::string str;
	while (std::getline(in, line)) {
		str.append(line+"\n");
	}
	in.close();

	doc_.Parse<0>(str.c_str());
	if (doc_.HasParseError()) {
		std::cerr << "parse " << jsonpath << " failed, err " << doc_.GetParseError() << std::endl;
		return false;
	}

	if (!doc_["server_name"].IsString()) {
		
	}
	
	std::cout << "load " << jsonpath << " success" << std::endl;
	return true;
}

void ConfigLoader::close()
{
}
