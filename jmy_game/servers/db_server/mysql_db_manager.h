#pragma once

#include "mysql_connector_pool.h"

class MysqlDBManager
{
public:
	MysqlDBManager();
	~MysqlDBManager();

	bool init(const MysqlDatabaseConfig& config);
	void clear();

	template <typename F1>
	bool insertRecord(const char* table_name, const char* field1_name, const F1& field1_value);
	template <typename... Args>
	bool updateRecord(const char* table_name, Args... args);
	template <typename KeyType>
	bool deleteRecord(const char* table_name, const char* key_name, const KeyType& key);
	template <typename KeyType>
	bool selectRecord(const char* table_name, const char* key_name, const KeyType& key);
	
private:
	MysqlConnectorPool pool_;
};
