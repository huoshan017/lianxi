#pragma once

#include <string>

enum MysqlTableFieldType {
	MYSQL_FIELD_TYPE_NONE = 0,
	MYSQL_FIELD_TYPE_TINYINT = 1,			// TINYINT
	MYSQL_FIELD_TYPE_SMALLINT = 2,		// SMALLINT
	MYSQL_FIELD_TYPE_MEDIUMINT = 3,		// MEDIUMINT
	MYSQL_FIELD_TYPE_INT = 4,				// INT
	MYSQL_FIELD_TYPE_BIGINT = 5,			// BIGINT
	MYSQL_FIELD_TYPE_FLOAT = 6,			// FLOAT
	MYSQL_FIELD_TYPE_DOUBLE = 7,			// DOUBLE
	MYSQL_FIELD_TYPE_DATE = 8,			// DATE
	MYSQL_FIELD_TYPE_DATETIME = 9,		// DATETIME
	MYSQL_FIELD_TYPE_TIMESTAMP = 10,		// TIMESTAMP
	MYSQL_FIELD_TYPE_TIME = 11,			// TIME
	MYSQL_FIELD_TYPE_YEAR = 12,			// YEAR
	MYSQL_FIELD_TYPE_CHAR = 13,			// CHAR
	MYSQL_FIELD_TYPE_VARCHAR = 14,		// VARCHAR
	MYSQL_FIELD_TYPE_TINYBLOB = 15,		// TINYBLOB
	MYSQL_FIELD_TYPE_TINYTEXT = 16,		// TINYTEXT
	MYSQL_FIELD_TYPE_BLOB = 17,			// BLOB
	MYSQL_FIELD_TYPE_TEXT = 18,			// TEXT
	MYSQL_FIELD_TYPE_MEDIUMBLOB = 19,		// MEDIUMBLOB
	MYSQL_FIELD_TYPE_MEDIUMTEXT = 20,		// MEDIUMTEXT
	MYSQL_FIELD_TYPE_LONGBLOB = 21,		// LONGBLOB
	MYSQL_FIELD_TYPE_LONGTEXT = 22,		// LONGTEXT
	MYSQL_FIELD_TYPE_ENUM = 23,			// ENUM
	MYSQL_FIELD_TYPE_SET = 24,			// SET
};

enum {
	MYSQL_FIELD_DEFAULT_LENGTH = 0,
	MYSQL_FIELD_DEFAULT_LENGTH_TINYINT = 4,
	MYSQL_FIELD_DEFAULT_LENGTH_SMALLINT = 6,
	MYSQL_FIELD_DEFAULT_LENGTH_MEDIUMINT = 8,
	MYSQL_FIELD_DEFAULT_LENGTH_INT = 11,
	MYSQL_FIELD_DEFAULT_LENGTH_BIGINT = 20,
	MYSQL_FIELD_DEFAULT_LENGTH_FLOAT = 11,
	MYSQL_FIELD_DEFAULT_LENGTH_DOUBLE = 20,
	MYSQL_FIELD_DEFAULT_LENGTH_DATE = 10,
	MYSQL_FIELD_DEFAULT_LENGTH_DATETIME = 19,
	MYSQL_FIELD_DEFAULT_LENGTH_TIMESTAMP = 6,
	MYSQL_FIELD_DEFAULT_LENGTH_TIME = 8,
	MYSQL_FIELD_DEFAULT_LENGTH_YEAR = 4,
	MYSQL_FIELD_DEFAULT_LENGTH_CHAR = 255,
	MYSQL_FIELD_DEFAULT_LENGTH_VARCHAR = 65530,
	MYSQL_FIELD_DEFAULT_LENGTH_TINYBLOB = 255,
	MYSQL_FIELD_DEFAULT_LENGTH_TINYTEXT = 255,
	MYSQL_FIELD_DEFAULT_LENGTH_BLOB = 65535,
	MYSQL_FIELD_DEFAULT_LENGTH_TEXT = 65535,
	MYSQL_FIELD_DEFAULT_LENGTH_MEDIUMBLOB = 16777215,
	MYSQL_FIELD_DEFAULT_LENGTH_MEDIUMTEXT = 16777215,
	MYSQL_FIELD_DEFAULT_LENGTH_LONGBLOB = 4294967295,
	MYSQL_FIELD_DEFAULT_LENGTH_LONGTEXT = 4294967295,
};

enum MysqlTableCreateFlag {
	MYSQL_TABLE_CREATE_ZEROFILL = 1,
	MYSQL_TABLE_CREATE_UNSIGNED = 2,
	MYSQL_TABLE_CREATE_AUTOINCREMENT = 4,
	MYSQL_TABLE_CREATE_NULL = 8,
	MYSQL_TABLE_CREATE_NOT_NULL = 16,
	MYSQL_TABLE_CREATE_DEFAULT = 32,
};

enum MysqlTableIndexType {
	MYSQL_INDEX_TYPE_NONE = 0,
	MYSQL_INDEX_TYPE_NORMAL = 1,
	MYSQL_INDEX_TYPE_UNIQUE = 2,
	MYSQL_INDEX_TYPE_FULLTEXT = 3,
};

enum MysqlEngineType {
	MYSQL_ENGINE_MYISAM = 0,
	MYSQL_ENGINE_INNODB = 1,
};

struct MysqlTableFieldInfo {
	const char* name;
	MysqlTableFieldType field_type;
	int type_length;
	int index_type;
	int create_flags;
};

struct MysqlTableInfo {
	const char* name;
	int primary_key_index;
	const char* primary_key;
	const MysqlTableFieldInfo* fields_info;
	const int fields_num;
	const MysqlEngineType engine_type;
};

struct MysqlDatabaseConfig {
	const char* dbname;
	const MysqlTableInfo* tables_info;
	const int tables_num;
};

typedef int (*mysql_cmd_callback_func)(MysqlConnector::Result& result, void* param, long param_l);

template <typename T>
struct MysqlFieldNameValue {
	std::string field_name;
	T field_value;
	MysqlFieldNameValue() {}
	MysqlFieldNameValue(const char* fn, const T& fv) : field_name(fn), field_value(fv) {}
	MysqlFieldNameValue(const std::string& fn, const T& fv) : field_name(fn), field_value(fv) {}
	MysqlFieldNameValue(MysqlFieldNameValue&& fp) : field_name(std::move(fp.field_name)), field_value(std::move(fp.field_value)) {}
	MysqlFieldNameValue& operator=(MysqlFieldNameValue&& fp) {
		field_name = std::move(fp.field_name);
		field_value = std::move(fp.field_value);
		return *this;
	}
};

template <typename T>
struct MysqlFieldIndexValue {
	int field_index;
	T field_value;
	MysqlFieldIndexValue() : field_index(0) {}
	MysqlFieldIndexValue(int fi, const T& fv) : field_index(fi), field_value(fv) {}
	MysqlFieldIndexValue(MysqlFieldIndexValue&& f) : field_index(f.field_index), field_value(f.field_value) {}
	MysqlFieldIndexValue& operator=(MysqlFieldIndexValue&& f) {
		field_index = f.field_index;
		field_value = f.field_value;
		return *this;
	}
};
