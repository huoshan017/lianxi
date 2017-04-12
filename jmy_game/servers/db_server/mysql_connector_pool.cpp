#include "mysql_connector_pool.h"
#include "../common/util.h"
#include <thread>
#include <chrono>

MysqlConnectorPool::MysqlConnectorPool() : curr_read_index_(0), curr_write_index_(0)
{
}

MysqlConnectorPool::~MysqlConnectorPool()
{
	close();
}

static bool one_connector_init(MysqlConnector& conn, const MysqlConnPoolConfig& config) {
	if (!conn.init()) return false;
	if (!conn.connect(config.host, config.port, config.user, config.passwd, config.dbname))
		return false;
	return true;
}

static void connector_read_func(MysqlConnectorPool::ConnectorInfo & conn) {
	while (true) {
		if (conn.cmd_list.size() > 0) {
			MysqlConnectorPool::CmdInfo& ci = conn.cmd_list.front();
			if (!conn.connector.real_select(ci.sql, ci.sql_len)) {
				ServerLogError("real select failed");
				break;
			}
			const MysqlConnector::Result& r = conn.connector.get_result();
			conn.cmd_list.pop_front();
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

static void connector_write_func(MysqlConnectorPool::ConnectorInfo& conn) {
	while (true) {
		if (conn.cmd_list.size() > 0) {
			MysqlConnectorPool::CmdInfo& ci = conn.cmd_list.front();
			if (!conn.connector.real_query(ci.sql, ci.sql_len)) {
				ServerLogError("real query failed");
				break;
			}
			const MysqlConnector::Result& r = conn.connector.get_result();
			conn.cmd_list.pop_front();
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

bool MysqlConnectorPool::init(const MysqlConnPoolConfig& config)
{
	read_connectors_.resize(config.read_conn_size);
	std::vector<ConnectorInfo>::iterator it = read_connectors_.begin();
	for (; it!=read_connectors_.end(); ++it) {
		MysqlConnector& conn = it->connector;
		if (!one_connector_init(conn, config)) {
			ServerLogError("read connector init failed");
			return false;
		}
		threads_.create_thread(std::bind(connector_read_func, conn));
	}
	write_connectors_.resize(config.write_conn_size);
	it = write_connectors_.begin();
	for (; it!=write_connectors_.end(); ++it) {
		if (!one_connector_init(it->connector, config)) {
			ServerLogError("write connector init failed");
			return false;
		}
		threads_.create_thread(std::bind(connector_write_func, *it));
	}
	return true;
}

void MysqlConnectorPool::close()
{
	threads_.join_all();
	std::vector<ConnectorInfo>::iterator it = read_connectors_.begin();
	for (; it!=read_connectors_.end(); ++it) {
		it->connector.close();
		it->cmd_list.clear();
	}
	read_connectors_.clear();
	it = write_connectors_.begin();
	for (; it!=write_connectors_.end(); ++it) {
		it->connector.close();
		it->cmd_list.clear();
	}
	write_connectors_.clear();
}

