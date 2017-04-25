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

inline const char* mysql_get_field_type_format(MysqlTableFieldType ft, int flags) {
	if (ft == MYSQL_FIELD_TYPE_TINYINT || ft == MYSQL_FIELD_TYPE_SMALLINT ||
		ft == MYSQL_FIELD_TYPE_MEDIUMINT || ft == MYSQL_FIELD_TYPE_INT) {
		if (flags & MYSQL_TABLE_CREATE_UNSIGNED) {
			return "%u";
		} else {
			return "%d";
		}
	} else if (ft == MYSQL_FIELD_TYPE_BIGINT) {
		if (flags & MYSQL_TABLE_CREATE_UNSIGNED) {
			return "%lld";
		} else {
			return "%llu";
		}
	} else if (IS_MYSQL_TEXT_TYPE(ft) || IS_MYSQL_BINARY_TYPE(ft)) {
		return "%s";
	} else {
		return nullptr;
	}
}
