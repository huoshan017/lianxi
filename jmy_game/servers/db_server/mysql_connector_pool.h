#pragma once

#include "mysql_connector.h"
#include <vector>
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
		: host(nullptr), port(3306), user(nullptr), passwd(nullptr), dbname(nullptr), read_conn_size(8), write_conn_size(1)
	{
	}
	MysqlConnPoolConfig(char* h, char* u, char* p, char* db)
		: host(h), port(3306), user(u), passwd(p), dbname(db), read_conn_size(8), write_conn_size(1)
	{
	}
};

typedef int (mysql_cmd_callback_func)(void* param, long param_l);

class MysqlConnectorPool
{
public:
	MysqlConnectorPool();
	~MysqlConnectorPool();
	bool init(const MysqlConnPoolConfig& config);
	void close();

	struct CmdInfo {
		CmdInfo(const char* s, unsigned short l) {
			sql = (char*)jmy_mem_malloc(l);
			std::memcpy(sql, s, l);
			sql_len = l;
		}
		~CmdInfo() { clear(); }
		void clear() {
			if (sql) {
				jmy_mem_free(sql);
				sql = nullptr;
				sql_len = 0;
			}
		}
		char* sql;
		unsigned short sql_len;
		mysql_cmd_callback_func func;
		void* param; long param_l;
	};

	struct ConnectorInfo {
		MysqlConnector connector;
		std::list<CmdInfo> cmd_list;
	};

	bool push_read_cmd(CmdInfo&& info);
	bool push_write_cmd(CmdInfo&& info);
	int run();

private:
	std::vector<ConnectorInfo> write_connectors_;
	std::vector<ConnectorInfo> read_connectors_;
	boost::thread_group threads_;
	int curr_read_index_;
	int curr_write_index_;
};
