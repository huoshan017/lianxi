#include "../thirdparty/include/rapidjson/document.h"
#include "../thirdparty/include/rapidjson/stringbuffer.h"
#include "../thirdparty/include/rapidjson/writer.h"
#include "generate_source.h"
#include "generate_manager.h"
#include <iostream>
#include <vector>
#include <string>

#define CONFIG_FILE "../game_server/csv_list.json"

static std::vector<std::string> csv_list;

static bool load_config(const char* jsonpath) {
	std::ifstream in;

	in.open(jsonpath, std::ifstream::in);
	if (!in.is_open()) {
		std::cout << "failed to open " << jsonpath << std::endl;
		return false;
	}

	std::string line;
	std::string str;
	while (std::getline(in, line)) {
		str.append(line+"\n");
	}
	in.close();

	rapidjson::Document doc;
	doc.Parse<0>(str.c_str());
	if (doc.HasParseError()) {
		std::cout << "parse " << jsonpath << " failed, err " << doc.GetParseError() << std::endl;
		return false;
	}

	rapidjson::Value v = doc.GetArray();
	if (!v.IsArray()) {
		std::cout << "type is not array" << std::endl;
		return false;
	}

	int s = v.Size();
	for (int i=0; i<s; ++i) {
		if (!v[i].IsString()) {
			std::cout << "array member is not string" << std::endl;
			return false;
		}
		csv_list.push_back(v[i].GetString());
	}

	return true;
}

int main(int argc, char** argv)
{
	if (argc < 2) {
		std::cout << "argument count is invalid" << std::endl;
		return -1;
	}

	std::string csv_root = argv[1];

	if (!load_config(CONFIG_FILE)) {
		std::cout << "load config " << CONFIG_FILE << " failed" << std::endl;
		return -1;
	}

	GenerateManager mgr;
	std::vector<std::string>::iterator it = csv_list.begin();
	for (; it!= csv_list.end(); ++it) {
		GenerateSource gs;
		std::string file_path = csv_root + "/" + *it;
		if (!gs.init(file_path.c_str())) {
			std::cout << "init file " << file_path.c_str() << " failed" << std::endl;
			return -1;
		}
		gs.close();
		std::string file_name = gs.getFileName() + ".h";
		std::string class_name = gs.getFileName() + "_table";
		mgr.loadOne((*it).c_str(), file_name.c_str(), class_name.c_str());
	}

	mgr.generate();
	mgr.close();

	return 0;
}
