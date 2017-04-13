#pragma once

#include "mysql/mysql.h"
#include <string>
#include <unordered_map>
#include <list>

class MysqlConnector
{
public:
	MysqlConnector();
	~MysqlConnector();

	bool init();
	bool connect(const char* host, const char* user, const char* password);
	bool connect(const char* host, unsigned short port, const char* user, const char* password);
	bool connect(const char* host, const char* user, const char* password, const char* dbname);
	bool connect(const char* host, unsigned short port, const char* user, const char* passwd, const char* dbname);
	void close();

	bool create_db(const char* db_name);
	bool use_db(const char* db_name);
	bool drop_db(const char* db_name);
	bool query(const char* stmt_str);
	bool real_query(const char* stmt_str, unsigned long length);
	bool select(const char* stmt_str);
	bool real_select(const char* stmt_str, unsigned long length);

	struct Result {
		MYSQL_RES* res;
		MYSQL_ROW row;
		int nfields;
		typedef std::unordered_map<const char*, const char*> res_value_type;
		res_value_type res_values_;

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
		int num_rows() {
			return mysql_num_rows(res);
		}
	};

	struct ResultList {
		std::list<Result> list_;
	};

	const Result& get_result() const { return res_; }
	const ResultList& get_result_list() const { return res_list_; }
	bool to_next_result();

	unsigned long real_escape_string(char* to, const char* from, unsigned long length);

private:
	bool store_result();

private:
	MYSQL* handle_;
	Result res_;
	ResultList res_list_;
	char buf_[1024*8];
};
