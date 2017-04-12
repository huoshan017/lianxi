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

static void connector_read_func(MysqlConnectorPool::ConnectorInfo* conn) {
	MysqlConnectorPool::CmdInfo ci;
	while (true) {
		if (conn->pop(ci)) {
			if (!conn->connector.real_select(ci.sql, ci.sql_len)) {
				ServerLogError("real select failed");
				break;
			}
			MysqlConnector::Result& r = conn->get_result();
			MysqlConnectorPool::ResultInfo ri(r);
			conn->push_res(ri);
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

static void connector_write_func(MysqlConnectorPool::ConnectorInfo* conn) {
	MysqlConnectorPool::CmdInfo ci;
	while (true) {
		if (conn->pop(ci)) {
			if (!conn->connector.real_query(ci.sql, ci.sql_len)) {
				ServerLogError("real query failed");
				break;
			}
			MysqlConnector::Result& r = conn->get_result();
			MysqlConnectorPool::ResultInfo ri(r);
			conn->push_res(ri);
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

bool MysqlConnectorPool::init(const MysqlConnPoolConfig& config)
{
	size_t i = 0;
	ConnectorInfo* ci = nullptr; 
	read_connectors_.resize(config.read_conn_size);
	for (; i<read_connectors_.size(); ++i) {
		ci = jmy_mem_malloc<ConnectorInfo>();
		read_connectors_[i] = ci;
		if (!one_connector_init((ci->connector), config)) {
			ServerLogError("read connector init failed");
			return false;
		}
		threads_.create_thread(std::bind(connector_read_func, ci));
	}

	write_connectors_.resize(config.write_conn_size);
	for (i=0; i<write_connectors_.size(); ++i) {
		ci = jmy_mem_malloc<ConnectorInfo>();
		write_connectors_[i] = ci;
		if (!one_connector_init(ci->connector, config)) {
			ServerLogError("write connector init failed");
			return false;
		}
		threads_.create_thread(std::bind(connector_write_func, ci));
	}
	return true;
}

void MysqlConnectorPool::close()
{
	threads_.join_all();
	std::vector<ConnectorInfo*>::iterator it = read_connectors_.begin();
	for (; it!=read_connectors_.end(); ++it) {
		(*it)->clear();
	}
	read_connectors_.clear();
	it = write_connectors_.begin();
	for (; it!=write_connectors_.end(); ++it) {
		(*it)->clear();
	}
	write_connectors_.clear();
}

bool MysqlConnectorPool::push_read_cmd(CmdInfo& info)
{
	if (curr_read_index_ > (int)(read_connectors_.size()-1)) {
		curr_read_index_ = 0;
	} else {
		curr_read_index_ += 1;
	}
	read_connectors_[curr_read_index_]->push(info);
	return true;
}

bool MysqlConnectorPool::push_write_cmd(CmdInfo& info)
{
	if (curr_write_index_ > (int)(write_connectors_.size()-1)) {
		curr_read_index_ = 0;
	} else {
		curr_read_index_ += 1;
	}
	write_connectors_[curr_write_index_]->push(info);
	return true;
}

int MysqlConnectorPool::run()
{
	ResultInfo ri;
	ConnectorInfo* ci = nullptr;
	size_t i = 0;
	for (; i<read_connectors_.size(); ++i) {
		ci = read_connectors_[i];
		if (ci->pop_res(ri)) {
			ri.cb_func(ri.param, ri.param_l);
		}
	}
	for (; i<write_connectors_.size(); ++i) {
		ci = write_connectors_[i];
		if (ci->pop_res(ri)) {
			ri.cb_func(ri.param, ri.param_l);
		}
	}
	return 0;
}
