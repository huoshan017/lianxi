#pragma once

class MysqlConnector;
struct MysqlDatabaseConfig;
struct MysqlTableInfo;
struct MysqlTableFieldInfo;

class MysqlConfigLoader
{
public:
	MysqlConfigLoader(MysqlConnector* connector);
	~MysqlConfigLoader();

	void setConnector(MysqlConnector* connector) { connector_ = connector; }
	bool load(const MysqlDatabaseConfig& config);
	void clear();

private:
	bool loadTable(const MysqlTableInfo& table_info);
	bool dropTable(const char* table_name);
	const char* get_field_type(const MysqlTableFieldInfo& field_info);
	const char* get_field_create_flags(const MysqlTableFieldInfo& field_info);
	bool add_field(const char* table_name, const MysqlTableFieldInfo& field_info);
	bool remove_field(const char* table_name, const char* field_name);
	bool rename_field(const char* table_name, const char* old_field_name, const char* new_field_name);
	bool modify_field_attr(const char* table_name, const MysqlTableFieldInfo& field_info);

private:
	MysqlConnector* connector_;
	char buf_[1024*8];
};
