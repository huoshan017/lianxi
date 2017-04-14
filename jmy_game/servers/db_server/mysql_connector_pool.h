#pragma once

#include "mysql_connector.h"
#include <vector>
#include <mutex>
#include <boost/thread/thread.hpp>
#include "../libjmy/jmy_mem.h"

enum { MYSQL_POOL_READ_CONN_SIZE = 8 };
enum { MYSQL_POOL_WRITE_CONN_SIZE = 1 };

struct MysqlConnPoolConfig {
	char* host;
	unsigned short port;
	char* user;
	char* passwd;
	char* dbname;
	uint8_t read_conn_size;
	uint8_t write_conn_size;
	MysqlConnPoolConfig()
		: host(nullptr), port(3306), user(nullptr), passwd(nullptr), dbname(nullptr),
		read_conn_size(MYSQL_POOL_READ_CONN_SIZE), write_conn_size(MYSQL_POOL_WRITE_CONN_SIZE)
	{
	}
	MysqlConnPoolConfig(char* h, char* u, char* p, char* db)
		: host(h), port(3306), user(u), passwd(p), dbname(db),
		read_conn_size(MYSQL_POOL_READ_CONN_SIZE), write_conn_size(MYSQL_POOL_WRITE_CONN_SIZE)
	{
	}
};

typedef int (*mysql_cmd_callback_func)(void* param, long param_l);

class MysqlConnectorPool
{
public:
	MysqlConnectorPool();
	~MysqlConnectorPool();
	bool init(const MysqlConnPoolConfig& config);
	void close();

	struct CmdInfo {
		CmdInfo() : sql(nullptr), sql_len(0), callback_func(nullptr), param(nullptr), param_l(0) {
		}
		CmdInfo(const char* s, unsigned short l) : callback_func(nullptr), param(nullptr), param_l(0) {
			sql = (char*)jmy_mem_malloc(l);
			std::memcpy(sql, s, l);
			sql_len = l;
		}
		CmdInfo(CmdInfo&& ci) : sql(ci.sql), sql_len(ci.sql_len), callback_func(ci.callback_func), param(ci.param), param_l(ci.param_l) {
			ci.sql = nullptr;
			ci.sql_len = 0;
			ci.callback_func = nullptr;
			ci.param = 0;
			ci.param_l = 0;
		}
		CmdInfo& operator=(CmdInfo&& ci) {
			sql = ci.sql;
			sql_len = ci.sql_len;
			callback_func = ci.callback_func;
			param = ci.param;
			param_l = ci.param_l;
			return *this;
		}
		~CmdInfo() { clear(); }
		void clear() {
			if (sql) {
				jmy_mem_free(sql);
				sql = nullptr;
				sql_len = 0;
			}
			callback_func = nullptr;
			param = nullptr;
			param_l = 0;
		}
		char* sql;
		unsigned short sql_len;
		mysql_cmd_callback_func callback_func;
		void* param;
		long param_l;
	};

	struct ResultInfo {
		MysqlConnector::Result res;
		mysql_cmd_callback_func cb_func;
		void* param;
		long param_l;
		ResultInfo() : cb_func(nullptr), param(nullptr), param_l(0) {}
		ResultInfo(MysqlConnector::Result& r)
			: res(std::move(r)), cb_func(nullptr), param(nullptr), param_l(0) {
		}
		ResultInfo(ResultInfo&& ri)
			: res(std::move(ri.res)), cb_func(ri.cb_func), param(ri.param), param_l(ri.param_l) {
		}
		ResultInfo& operator=(ResultInfo&& ri) {
			res = std::move(ri.res);
			cb_func = ri.cb_func;
			param = ri.param;
			param_l = ri.param_l;
			return *this;
		}
	};

	struct ConnectorInfo {
		MysqlConnector connector;
		std::list<CmdInfo> cmd_list;
		std::mutex cmd_mtx_;
		std::list<ResultInfo> res_list;
		std::mutex res_mtx_;
		void clear() {
			res_list.clear();
			connector.close();
			cmd_list.clear();
		}
		void push(CmdInfo& info) {
			std::lock_guard<std::mutex> lk(cmd_mtx_);
			cmd_list.push_back(std::move(info));
		}
		bool pop(CmdInfo& info) {
			if (cmd_list.size() == 0)
				return false;
			std::lock_guard<std::mutex> lk(cmd_mtx_);
			if (cmd_list.size() == 0)
				return false;
			info = std::move(cmd_list.front());
			cmd_list.pop_front();
			return true;
		}
		void push_res(ResultInfo& res) {
			std::lock_guard<std::mutex> lk(res_mtx_);
			res_list.push_back(std::move(res));
		}
		bool pop_res(ResultInfo& res) {
			if (res_list.size() == 0)
				return false;
			std::lock_guard<std::mutex> lk(res_mtx_);
			if (res_list.size() == 0)
				return false;
			res = std::move(res_list.front());
			res_list.pop_front();
			return true;
		}
		MysqlConnector::Result& get_result() {
			return const_cast<MysqlConnector::Result&>(connector.get_result());
		}
	};

	bool push_read_cmd(CmdInfo& info);
	bool push_write_cmd(CmdInfo& info);
	int run();

private:
	std::vector<ConnectorInfo*> write_connectors_;
	std::vector<ConnectorInfo*> read_connectors_;
	boost::thread_group threads_;
	int curr_read_index_;
	int curr_write_index_;
};
