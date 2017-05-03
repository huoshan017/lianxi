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

template <typename FieldType>
inline bool mysql_get_field_value_format(MysqlTableFieldType ft, int flags, FieldType value, char* format_buf, int format_buf_len) { 
	(void)ft;
	(void)flags;
	(void)value;
	(void)format_buf;
	(void)format_buf_len;
	return false;
}

template <>
inline bool mysql_get_field_value_format<int>(MysqlTableFieldType ft, int flags, int value, char* format_buf, int format_buf_len) {
	if (ft == MYSQL_FIELD_TYPE_TINYINT || ft == MYSQL_FIELD_TYPE_SMALLINT ||
		ft == MYSQL_FIELD_TYPE_MEDIUMINT || ft == MYSQL_FIELD_TYPE_INT) {
		if (flags & MYSQL_TABLE_CREATE_UNSIGNED) {
			std::snprintf(format_buf, format_buf_len, "%u", value);
			return true;
		} else {
			std::snprintf(format_buf, format_buf_len, "%d", value);
			return true;
		}
	}
	return false;
}

template <>
inline bool mysql_get_field_value_format<long long>(MysqlTableFieldType ft, int flags, long long value, char* format_buf, int format_buf_len)
{
	if (ft == MYSQL_FIELD_TYPE_BIGINT) {
		if (flags & MYSQL_TABLE_CREATE_UNSIGNED) {
			std::snprintf(format_buf, format_buf_len, "%lld", value);
			return true;
		} else {
			std::snprintf(format_buf, format_buf_len, "%llu", value);
			return true;
		}
	}
	return false;
}

template <>
inline bool mysql_get_field_value_format<double>(MysqlTableFieldType ft, int flags, double value, char* format_buf, int format_buf_len)
{
	(void)flags;
	if (ft == MYSQL_FIELD_TYPE_FLOAT || ft == MYSQL_FIELD_TYPE_DOUBLE) {
		std::snprintf(format_buf, format_buf_len, "%f", value);
		return true;
	}
	return false;
}

template <>
inline bool mysql_get_field_value_format<const char*>(MysqlTableFieldType ft, int flags, const char* value, char* format_buf, int format_buf_len)
{
	(void)flags;
	if (IS_MYSQL_TEXT_TYPE(ft) || IS_MYSQL_BINARY_TYPE(ft)) {
		std::snprintf(format_buf, format_buf_len, "%s", value);
		return true;
	}
	return false;
}

template <>
inline bool mysql_get_field_value_format<const std::string&>(MysqlTableFieldType ft, int flags, const std::string& value, char* format_buf, int format_buf_len)
{
	(void)flags;
	if (IS_MYSQL_TEXT_TYPE(ft) || IS_MYSQL_BINARY_TYPE(ft)) {
		std::snprintf(format_buf, format_buf_len, "%s", value.c_str());
		return true;
	}
	return false;
}

template <>
inline bool mysql_get_field_value_format<std::string&>(MysqlTableFieldType ft, int flags, std::string& value, char* format_buf, int format_buf_len)
{
	(void)flags;
	if (IS_MYSQL_TEXT_TYPE(ft) || IS_MYSQL_BINARY_TYPE(ft)) {
		std::snprintf(format_buf, format_buf_len, "%s", value.c_str());
		return true;
	}
	return false;
}
