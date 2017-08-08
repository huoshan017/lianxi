#pragma once

#include "mysql/mysql.h"
#include "../common/util.h"
#include <string>
#include <unordered_map>
#include <list>
#include <functional>

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
	bool read_query(const char* stmt_str);
	bool real_read_query(const char* stmt_str, unsigned long length);

	struct Result {
		typedef std::unordered_map<const char*, const char*> res_value_type;

		MYSQL_RES* res;
		MYSQL_ROW row;
		int nfields;
		res_value_type res_values_;
		int res_err;

		Result() : res(nullptr), row(nullptr), nfields(0), res_err(0) {}
		Result(MYSQL_RES* r) : res(r) {
			nfields = mysql_num_fields(res);
		}
		Result(MYSQL* h) : res(nullptr), res_err(0) {
			res = mysql_store_result(h);
			nfields = mysql_num_fields(res);
		}
		Result(int err) : res(nullptr), row(nullptr), nfields(0), res_err(err) {
		}
		Result(Result&& r) : res(r.res), row(r.row), nfields(r.nfields), res_err(r.res_err) {
			r.res = nullptr;
			r.row = nullptr;
			r.nfields = 0;
			r.res_err = 0;
		}
		Result& operator=(Result&& r) {
			res = r.res;
			row = r.row;
			nfields = r.nfields;
			res_err = r.res_err;
			r.res = nullptr;
			r.row = nullptr;
			r.nfields = 0;
			r.res_err = 0;
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
		void init(int err) { clear(); nfields = 0;
			res_err = err;
		}
		void clear() {
			if (res) {
				mysql_free_result(res);
				res = nullptr;
			}
		}
		int num_fields() const {
			return nfields;
		}
		MYSQL_ROW fetch() {
			if (!res) return nullptr;
			row = mysql_fetch_row(res);
			return row;
		}
		char* get(int index) {
			return row[index];
		}
		int num_rows() const {
			return mysql_num_rows(res);
		}
		bool is_empty() const {
			return res == nullptr;
		}
		const unsigned long* row_lengths() const {
			if (!res) return nullptr;
			const unsigned long* r = mysql_fetch_lengths(res);
			return r;
		}
	};

	struct ResultList {
		std::list<Result> list_;
	};

	const Result& get_result() const { return res_; }
	const ResultList& get_result_list() const { return res_list_; }
	bool to_next_result();

	unsigned int real_escape_string(char* to, const char* from, unsigned int length);
	unsigned int real_escape_string(char** to, const char* from, unsigned int length);
	unsigned int get_escape_string_buff_len() const { return MAX_BUFFER_SIZE-1; }

private:
	bool store_result();

private:
	MYSQL* handle_;
	Result res_;
	ResultList res_list_;
	enum { MAX_BUFFER_SIZE = 1024*16 };
	char buf_[MAX_BUFFER_SIZE];
};

//typedef int (*mysql_cmd_callback_func)(MysqlConnector::Result& result, void* param, long param_l);
typedef std::function<int(MysqlConnector::Result&)> mysql_cmd_callback_func;
