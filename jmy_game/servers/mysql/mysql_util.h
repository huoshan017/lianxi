#pragma once

#include "mysql_defines.h"
#include "mysql_connector.h"
#include <iostream>

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
		(type == MYSQL_FIELD_TYPE_BINARY || type == MYSQL_FIELD_TYPE_VARBINARY)

#define IS_MYSQL_BLOB_TYPE(type) \
		(type == MYSQL_FIELD_TYPE_TINYBLOB || \
		 type == MYSQL_FIELD_TYPE_MEDIUMBLOB || \
		 type == MYSQL_FIELD_TYPE_BLOB || \
		 type == MYSQL_FIELD_TYPE_LONGBLOB)

#define IS_MYSQL_TIME_TYPE(type) \
		(type == MYSQL_FIELD_TYPE_DATE \
		 type == MYSQL_FIELD_TYPE_DATETIME \
		 type == MYSQL_FIELD_TYPE_TIME \
		 type == MYSQL_FIELD_TYPE_TIMESTAMP)

inline long mysql_field_default_length(MysqlTableFieldType field_type) {
	static long field_length_array[] = {
		MYSQL_FIELD_DEFAULT_LENGTH,
		MYSQL_FIELD_DEFAULT_LENGTH_TINYINT,
		MYSQL_FIELD_DEFAULT_LENGTH_SMALLINT,	
		MYSQL_FIELD_DEFAULT_LENGTH_MEDIUMINT,
		MYSQL_FIELD_DEFAULT_LENGTH_INT,
		MYSQL_FIELD_DEFAULT_LENGTH_BIGINT,
		MYSQL_FIELD_DEFAULT_LENGTH_FLOAT,
		MYSQL_FIELD_DEFAULT_LENGTH_DOUBLE,
		MYSQL_FIELD_DEFAULT_LENGTH_DATE,
		MYSQL_FIELD_DEFAULT_LENGTH_DATETIME,
		MYSQL_FIELD_DEFAULT_LENGTH_TIMESTAMP,
		MYSQL_FIELD_DEFAULT_LENGTH_TIME,
		MYSQL_FIELD_DEFAULT_LENGTH_YEAR,
		MYSQL_FIELD_DEFAULT_LENGTH_CHAR,
		MYSQL_FIELD_DEFAULT_LENGTH_VARCHAR,
		MYSQL_FIELD_DEFAULT_LENGTH_BINARY,
		MYSQL_FIELD_DEFAULT_LENGTH_VARBINARY,
		MYSQL_FIELD_DEFAULT_LENGTH_TINYBLOB,
		MYSQL_FIELD_DEFAULT_LENGTH_TINYTEXT,
		MYSQL_FIELD_DEFAULT_LENGTH_BLOB,
		MYSQL_FIELD_DEFAULT_LENGTH_TEXT,
		MYSQL_FIELD_DEFAULT_LENGTH_MEDIUMBLOB,
		MYSQL_FIELD_DEFAULT_LENGTH_MEDIUMTEXT,
		MYSQL_FIELD_DEFAULT_LENGTH_LONGBLOB,
		MYSQL_FIELD_DEFAULT_LENGTH_LONGTEXT
	};

	if ((int)field_type >= (int)(sizeof(field_length_array)/sizeof(field_length_array[0])))
		return -1;
	return field_length_array[field_type];
}

template <typename FieldType>
inline bool mysql_get_field_value_format(MysqlTableFieldType ft, int flags, const FieldType& value, char* format_buf, int format_buf_len) {
	(void)flags;
	if (IS_MYSQL_BINARY_TYPE(ft) || IS_MYSQL_BLOB_TYPE(ft)) {
		if (value.SerializeToArray(format_buf, format_buf_len)) {
			if (value.ByteSize() == 0) {
				std::snprintf(format_buf, format_buf_len, "\'\'");
			} else {
				format_buf[value.ByteSize()] = '\0';
			}
			return true;
		}
	}
	return false;
}

template <>
inline bool mysql_get_field_value_format<int>(MysqlTableFieldType ft, int flags, const int& value, char* format_buf, int format_buf_len) {
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
inline bool mysql_get_field_value_format<unsigned int>(MysqlTableFieldType ft, int flags, const unsigned int& value, char* format_buf, int format_buf_len) {
	if (ft == MYSQL_FIELD_TYPE_TINYINT || ft == MYSQL_FIELD_TYPE_SMALLINT ||
		ft == MYSQL_FIELD_TYPE_MEDIUMINT || ft == MYSQL_FIELD_TYPE_INT) {
		if (flags & MYSQL_TABLE_CREATE_UNSIGNED) {
			std::snprintf(format_buf, format_buf_len, "%u", value);
			return true;
		}
	}
	return false;
}

template <>
inline bool mysql_get_field_value_format<uint64_t>(MysqlTableFieldType ft, int flags, const uint64_t& value, char* format_buf, int format_buf_len)
{
	if (ft == MYSQL_FIELD_TYPE_BIGINT) {
		if (flags & MYSQL_TABLE_CREATE_UNSIGNED) {
			std::snprintf(format_buf, format_buf_len, "%lu", value);
			return true;
		} else {
			std::snprintf(format_buf, format_buf_len, "%ld", value);
			return true;
		}
	}
	return false;
}

template <>
inline bool mysql_get_field_value_format<int64_t>(MysqlTableFieldType ft, int flags, const int64_t& value, char* format_buf, int format_buf_len)
{
	if (ft == MYSQL_FIELD_TYPE_BIGINT) {
		if (flags & MYSQL_TABLE_CREATE_UNSIGNED) {
			std::snprintf(format_buf, format_buf_len, "%lu", value);
			return true;
		} else {
			std::snprintf(format_buf, format_buf_len, "%ld", value);
			return true;
		}
	}
	return false;
}

template <>
inline bool mysql_get_field_value_format<unsigned long long>(MysqlTableFieldType ft, int flags, const unsigned long long& value, char* format_buf, int format_buf_len)
{
	if (ft == MYSQL_FIELD_TYPE_BIGINT) {
		if (flags & MYSQL_TABLE_CREATE_UNSIGNED) {
			std::snprintf(format_buf, format_buf_len, "%llu", value);
			return true;
		} else {
			std::snprintf(format_buf, format_buf_len, "%lld", value);
			return true;
		}
	}
	return false;
}

template <>
inline bool mysql_get_field_value_format<double>(MysqlTableFieldType ft, int flags, const double& value, char* format_buf, int format_buf_len)
{
	(void)flags;
	if (ft == MYSQL_FIELD_TYPE_FLOAT || ft == MYSQL_FIELD_TYPE_DOUBLE) {
		std::snprintf(format_buf, format_buf_len, "%f", value);
		return true;
	}
	return false;
}

inline bool mysql_get_field_value_format(MysqlTableFieldType ft, int flags, const char* value, char* format_buf, int format_buf_len)
{
	(void)flags;
	if (IS_MYSQL_TEXT_TYPE(ft) || IS_MYSQL_BINARY_TYPE(ft)) {
		std::snprintf(format_buf, format_buf_len, "\'%s\'", value);
		return true;
	}
	return false;
}

template <>
inline bool mysql_get_field_value_format<std::string>(MysqlTableFieldType ft, int flags, const std::string& value, char* format_buf, int format_buf_len)
{
	(void)flags;
	if (IS_MYSQL_TEXT_TYPE(ft) || IS_MYSQL_BINARY_TYPE(ft)) {
		std::snprintf(format_buf, format_buf_len, "\'%s\'", value.c_str());
		return true;
	}
	return false;
}

inline int mysql_get_last_insert_id(MysqlConnector::Result& res) {
	if (res.res_err != 0) {
		std::cout << "get last insert id failed, err(" << res.res_err << ")" << std::endl;
		return -1;
	}

	if (res.num_rows() == 0 || res.is_empty()) {
		std::cout << "get last insert id failed, result is empty" << std::endl;
		return -1;	
	}

	char** datas = res.fetch();
	if (!datas) {
		std::cout << "cant get data from result" << std::endl;
		return -1;
	}

	return std::atoi(datas[0]);
}
