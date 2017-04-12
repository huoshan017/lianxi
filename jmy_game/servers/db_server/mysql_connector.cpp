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
	if (!mysql_init(handle_))
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

bool MysqlConnector::connect(const std::string& host, const std::string& user, const std::string& passwd, const std::string& dbname)
{
	return connect(host, 3306, user, passwd, dbname);
}

bool MysqlConnector::connect(const std::string& host, unsigned short port, const std::string& user, const std::string& passwd, const std::string& dbname)
{
	if (!mysql_real_connect(handle_, host.c_str(), user.c_str(), passwd.c_str(), dbname.c_str(), port, nullptr, 0)) {
		ServerLogError("connect mysql failed, err(%s)", mysql_error(handle_));
		return false;
	}
	ServerLogInfo("mysql connect success");
	return true;
}

bool MysqlConnector::query(const char* stmt_str)
{
	if (mysql_query(handle_, stmt_str) != 0) {
		ServerLogError("mysql_query error(%s)", mysql_error(handle_));
		return false;
	}
	return true;
}

bool MysqlConnector::real_query(const char* stmt_str, unsigned long length)
{
	if (mysql_real_query(handle_, stmt_str, length) != 0) {
		ServerLogError("mysql_real_query error(%s)", mysql_error(handle_));
		return false;
	}
	return true;
}

bool MysqlConnector::select(const char* stmt_str)
{
	if (!query(stmt_str)) {
		return false;
	}
	return store_result();
}

bool MysqlConnector::real_select(const char* stmt_str, unsigned long length)
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
