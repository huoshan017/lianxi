#pragma once

#include "../thirdparty/include/rapidjson/document.h"
#include "../thirdparty/include/rapidjson/stringbuffer.h"
#include "../thirdparty/include/rapidjson/writer.h"
#include <string>
#include <vector>
#include <list>

class DBFieldConfigParser
{
public:
	DBFieldConfigParser();
	~DBFieldConfigParser();

	bool load(const char* config_path);
	void clear();
	bool generate();

	struct FieldMemberInfo {
		std::string name;
		int index;
		std::string type;
		bool repeated;
	};

	struct FieldInfo {
		std::string name;
		std::vector<FieldMemberInfo> members;
	};

	FieldInfo* getFieldInfo(const std::string& field_name) {
		std::list<FieldInfo>::iterator it = field_list_.begin();
		for (; it!=field_list_.end(); ++it) {
			if ((*it).name == field_name) {
				return &(*it);
			}
		}
		return nullptr;
	}

	const std::list<FieldInfo>& getFieldInfoList() const { return field_list_; }

private:
	bool parse_field(const std::string& field_name, const rapidjson::Value& v, FieldInfo& field_info);
	bool parse_field_member(const std::string& field_name, const rapidjson::Value& v, const std::string& mem_name, FieldMemberInfo& mem_info);
	
private:
	rapidjson::Document doc_;
	std::string config_path_;
	std::list<FieldInfo> field_list_;
};
