#include "mysql_connector.h"
#include "../common/util.h"

MysqlConnector::MysqlConnector() : handle_(nullptr)
{
}

MysqlConnector::~MysqlConnector()
{
	close();
}

bool MysqlConnector::init()
{
	handle_ = mysql_init(handle_);
	if (!handle_)
		return false;
	return true;
}

void MysqlConnector::close()
{
	if (handle_) {
		mysql_close(handle_);
		handle_ = nullptr;
	}
	mysql_library_end();
}

bool MysqlConnector::connect(const char* host, const char* user, const char* password)
{
	return connect(host, user, password, nullptr);
}

bool MysqlConnector::connect(const char* host, unsigned short port, const char* user, const char* password)
{
	return connect(host, port, user, password, nullptr);
}

bool MysqlConnector::connect(const char* host, const char* user, const char* passwd, const char* dbname)
{
	return connect(host, 3306, user, passwd, dbname);
}

bool MysqlConnector::connect(const char* host, unsigned short port, const char* user, const char* passwd, const char* dbname)
{
	if (!mysql_real_connect(handle_, host, user, passwd, dbname, port, nullptr, 0)) {
		int err = mysql_errno(handle_);
		// specified database not exist
		if (err == 1049) {
			if (!mysql_real_connect(handle_, host, user, passwd, "", port, nullptr, 0)) {
				ServerLogError("connect mysql failed, err(%s)", mysql_error(handle_));
				return false;
			}
			if (!create_db(dbname)) {
				ServerLogError("create db %s error", dbname);
				return false;
			}
		} else {
			ServerLogError("connect mysql failed, err(%s)", mysql_error(handle_));
			return false;
		}
	}
	ServerLogInfo("mysql connect success");
	return true;
}

bool MysqlConnector::create_db(const char* db_name)
{
	std::snprintf(buf_, sizeof(buf_), "CREATE DATABASE IF NOT EXISTS %s", db_name);
	ServerLogInfo("create db sql: %s", buf_);
	return query(buf_);
}

bool MysqlConnector::use_db(const char* db_name)
{
	std::snprintf(buf_, sizeof(buf_), "USE DATABASE %s", db_name);
	return query(buf_);
}

bool MysqlConnector::drop_db(const char* db_name)
{
	std::snprintf(buf_, sizeof(buf_), "DROP DATABASE %s", db_name);
	return query(buf_);
}

bool MysqlConnector::query(const char* stmt_str)
{
	if (mysql_query(handle_, stmt_str) != 0) {
		res_.init(mysql_errno(handle_));
		ServerLogError("mysql_query error(%s)", mysql_error(handle_));
		return false;
	}
	return true;
}

bool MysqlConnector::real_query(const char* stmt_str, unsigned long length)
{
	if (mysql_real_query(handle_, stmt_str, length) != 0) {
		res_.init(mysql_errno(handle_));
		ServerLogError("mysql_real_query error(%s)", mysql_error(handle_));
		return false;
	}
	return true;
}

bool MysqlConnector::read_query(const char* stmt_str)
{
	if (!query(stmt_str)) {
		return false;
	}
	return store_result();
}

bool MysqlConnector::real_read_query(const char* stmt_str, unsigned long length)
{
	if (!real_query(stmt_str, length)) {
		return false;
	}
	return store_result();
}

bool MysqlConnector::store_result()
{
	MYSQL_RES* res = mysql_store_result(handle_);
	if (!res) {
		ServerLogError("mysql_store_result error(%s)", mysql_error(handle_));
		return false;
	}
	res_.init(res);
	return true;
}

bool MysqlConnector::to_next_result()
{
	if (mysql_next_result(handle_) != 0)
		return false;
	return true;
}

unsigned int MysqlConnector::real_escape_string(char* to, const char* from, unsigned int length)
{
	return mysql_real_escape_string(handle_, to, from, length);
}

unsigned int MysqlConnector::real_escape_string(char** to, const char* from, unsigned int length)
{
	if (!to || !(*to))
		return 0;
	if (2*length+1 > sizeof(buf_))
		return 0;
	unsigned int l = mysql_real_escape_string(handle_, buf_, from, length);
	*to = buf_;
	return l;
}
