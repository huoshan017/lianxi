#pragma once

#include "mysql_defines.h"

#define IS_MYSQL_INT_TYPE(type) \
		(type == MYSQL_FIELD_TYPE_TINYINT || \
		type == MYSQL_FIELD_TYPE_SMALLINT || \
		type == MYSQL_FIELD_TYPE_MEDIUMINT || \
		type == MYSQL_FIELD_TYPE_INT || \
		type == MYSQL_FIELD_TYPE_BIGINT)

#define IS_MYSQL_TEXT_TYPE(type) \
		(type == MYSQL_FIELD_TYPE_CHAR || \
		 type == MYSQL_FIELD_TYPE_VARCHAR || \
		 type == MYSQL_FIELD_TYPE_TEXT || \
		 type == MYSQL_FIELD_TYPE_TINYTEXT || \
		 type == MYSQL_FIELD_TYPE_MEDIUMTEXT || \
		 type == MYSQL_FIELD_TYPE_LONGTEXT)

#define IS_MYSQL_BINARY_TYPE(type) \
		(type == MYSQL_FIELD_TYPE_TINYBLOB || \
		 type == MYSQL_FIELD_TYPE_MEDIUMBLOB || \
		 type == MYSQL_FIELD_TYPE_BLOB || \
		 type == MYSQL_FIELD_TYPE_LONGBLOB)

#define IS_MYSQL_TIME_TYPE(type) \
		(type == MYSQL_FIELD_TYPE_DATE \
		 type == MYSQL_FIELD_TYPE_DATETIME \
		 type == MYSQL_FIELD_TYPE_TIME \
		 type == MYSQL_FIELD_TYPE_TIMESTAMP)

template <typename T>
char* mysql_format_field_type(char* buf, int buf_len,  MysqlTableFieldType field_type, const T& field_value)
{
	if (field_type == MYSQL_FIELD_TYPE_TINYINT || field_type == MYSQL_FIELD_TYPE_SMALLINT ||
		field_type == MYSQL_FIELD_TYPE_MEDIUMINT || field_type == MYSQL_FIELD_TYPE_INT) {
		std::snprintf(buf, buf_len, "");
	}
}
