#pragma once

#include "mysql_connector.h"
#include <vector>
#include <mutex>
#include <boost/thread/thread.hpp>
#include "../libjmy/jmy_mem.h"
#include "../common/util.h"
#include "mysql_defines.h"
#include "mysql_config_loader.h"

enum { MYSQL_POOL_READ_CONN_SIZE = 1 };
enum { MYSQL_POOL_WRITE_CONN_SIZE = 1 };

struct MysqlConnPoolConfig {
	char* host;
	unsigned short port;
	char* user;
	char* passwd;
	char* dbname;
	uint8_t read_conn_size;
	uint8_t write_conn_size;
	MysqlDatabaseConfig* database_config;
	MysqlConnPoolConfig()
		: host(nullptr), port(3306), user(nullptr), passwd(nullptr), dbname(nullptr),
		read_conn_size(MYSQL_POOL_READ_CONN_SIZE), write_conn_size(MYSQL_POOL_WRITE_CONN_SIZE),
		database_config(nullptr)
	{
	}
	MysqlConnPoolConfig(char* h, char* u, char* p, char* db, MysqlDatabaseConfig* db_conf)
		: host(h), port(3306), user(u), passwd(p), dbname(db),
		read_conn_size(MYSQL_POOL_READ_CONN_SIZE), write_conn_size(MYSQL_POOL_WRITE_CONN_SIZE),
		database_config(db_conf)
	{
	}
};

class MysqlConnectorPool
{
public:
	MysqlConnectorPool();
	~MysqlConnectorPool();
	bool init(const MysqlConnPoolConfig& config);
	void close();

	struct CmdInfo {
		CmdInfo() : sql(nullptr), sql_len(0), callback_func(nullptr), user_param(nullptr), user_param_l(0), write_cmd(false) {
		}
		CmdInfo(const char* s, unsigned short l) : callback_func(nullptr), user_param(nullptr), user_param_l(0), write_cmd(false) {
			sql = (char*)jmy_mem_malloc(l);
			std::memcpy(sql, s, l);
			sql_len = l;
		}
		CmdInfo(CmdInfo&& ci) : sql(ci.sql), sql_len(ci.sql_len), callback_func(ci.callback_func), user_param(ci.user_param), user_param_l(ci.user_param_l), write_cmd(ci.write_cmd) {
			ci.sql = nullptr;
			ci.sql_len = 0;
			ci.callback_func = nullptr;
			ci.user_param = 0;
			ci.user_param_l = 0;
			ci.write_cmd = false;
		}
		CmdInfo& operator=(CmdInfo&& ci) {
			sql = ci.sql;
			sql_len = ci.sql_len;
			callback_func = ci.callback_func;
			user_param = ci.user_param;
			user_param_l = ci.user_param_l;
			write_cmd = ci.write_cmd;
			ci.sql = nullptr;
			ci.sql_len = 0;
			ci.callback_func = nullptr;
			ci.user_param = nullptr;
			ci.user_param_l = 0;
			ci.write_cmd = false;
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
			user_param = nullptr;
			user_param_l = 0;
			write_cmd = false;
		}
		char* sql;
		unsigned short sql_len;
		mysql_cmd_callback_func callback_func;
		void* user_param;
		long user_param_l;
		bool write_cmd;
	};

	struct ResultInfo {
		MysqlConnector::Result res;
		mysql_cmd_callback_func cb_func;
		void* user_param;
		long user_param_l;
		ResultInfo() : cb_func(nullptr), user_param(nullptr), user_param_l(0) {}
		ResultInfo(MysqlConnector::Result& r)
			: res(std::move(r)), cb_func(nullptr), user_param(nullptr), user_param_l(0) {
		}
		ResultInfo(ResultInfo&& ri)
			: res(std::move(ri.res)), cb_func(ri.cb_func), user_param(ri.user_param), user_param_l(ri.user_param_l) {
			ri.cb_func = nullptr;
			ri.user_param = 0;
			ri.user_param_l = 0;
		}
		ResultInfo& operator=(ResultInfo&& ri) {
			res = std::move(ri.res);
			cb_func = ri.cb_func;
			user_param = ri.user_param;
			user_param_l = ri.user_param_l;
			ri.cb_func = nullptr;
			ri.user_param = 0;
			ri.user_param_l = 0;
			return *this;
		}
		~ResultInfo() {
			clear();
		}
		void clear() {
			res.clear();
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

	MysqlConnector* get_write_connector(int index) {
		if (index<0 || index>=(int)(write_connectors_.size()))
			return nullptr;
		return &(write_connectors_[index]->connector);
	}

	bool push_read_cmd(CmdInfo& info);
	bool push_write_cmd(CmdInfo& info);
	int run();

private:
	MysqlConfigLoader config_loader_;
	std::vector<ConnectorInfo*> write_connectors_;
	std::vector<ConnectorInfo*> read_connectors_;
	boost::thread_group threads_;
	int curr_read_index_;
	int curr_write_index_;
};
