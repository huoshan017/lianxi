#pragma once

#include <mysql/mysql.h>
#include <string>

class MysqlConnector
{
public:
	MysqlConnector();
	~MysqlConnector();

	bool init();
	bool connect(const std::string& host, const std::string& user, const std::string& password, const std::string& dbname);
	bool connect(const std::string& host, unsigned short port, const std::string& user, const std::string& passwd, const std::string& dbname);
	void close();

	bool query(const char* stmt_str);
	bool real_query(const char* stmt_str, unsigned long length);
	bool select(const char* stmt_str);
	bool real_select(const char* stmt_str, unsigned long length);

	struct Result {
		MYSQL_RES* res;
		MYSQL_ROW row;
		int nfields;

		Result() : res(nullptr), row(nullptr), nfields(0) {}
		Result(MYSQL_RES* r) : res(r) {
			nfields = mysql_num_fields(res);
		}
		Result(MYSQL* h) : res(nullptr) {
			res = mysql_store_result(h);
			nfields = mysql_num_fields(res);
		}
		Result(Result&& r) : res(r.res), row(r.row), nfields(r.nfields) {
			r.res = nullptr;
			r.row = nullptr;
			r.nfields = 0;
		}
		Result& operator=(Result&& r) {
			res = r.res;
			row = r.row;
			nfields = r.nfields;
			r.res = nullptr;
			r.row = nullptr;
			r.nfields = 0;
			return *this;
		}

		void init(MYSQL_RES* r) {
			clear();
			res = r;
			nfields = mysql_num_fields(res);
		}
		void init(MYSQL* h) {
			clear();
			res = mysql_store_result(h);
			nfields = mysql_num_fields(res);
		}
		void clear() {
			if (res) {
				mysql_free_result(res);
				res = nullptr;
			}
		}
		int num_fields() {
			return nfields;
		}
		char** fetch() {
			row = mysql_fetch_row(res);
			return row;
		}
		char* get(int index) {
			return row[index];
		}
	};

	const Result& get_result() const { return res_; }

private:
	bool store_result();

private:
	MYSQL* handle_;
	Result res_;
};
