#include "mysql_connector_pool.h"
#include "../common/util.h"
#include <thread>
#include <chrono>

MysqlConnectorPool::MysqlConnectorPool() : config_loader_(nullptr), curr_read_index_(0), curr_write_index_(0)
{
}

MysqlConnectorPool::~MysqlConnectorPool()
{
	close();
}

static bool one_connector_init(MysqlConnector& conn, const MysqlConnPoolConfig& config) {
	if (!conn.init()) return false;
	if (!conn.connect(config.host, config.user, config.passwd))
		return false;
	if (!conn.use_db(config.dbname)) {
		LogError("use db(%s) failed", config.dbname);
		return false;
	}
	return true;
}

static void connector_read_func(MysqlConnectorPool::ConnectorInfo* conn) {
	MysqlConnectorPool::CmdInfo ci;
	while (true) {
		if (conn->pop(ci)) {
			if (!conn->connector.real_read_query(ci.sql, ci.sql_len)) {
				LogWarn("real read query failed");
			}
			MysqlConnector::Result& r = conn->get_result();
			MysqlConnectorPool::ResultInfo ri(r);
			ri.cb_func = ci.callback_func;
			ri.user_param = ci.user_param;
			ri.user_param_l = ci.user_param_l;
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
				LogWarn("real query failed");
			}
			MysqlConnector::Result& r = conn->get_result();
			if (!r.is_empty()) {
				MysqlConnectorPool::ResultInfo ri(r);
				ri.cb_func = ci.callback_func;
				ri.user_param = ci.user_param;
				ri.user_param_l = ci.user_param_l;
				conn->push_res(ri);
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

bool MysqlConnectorPool::init(const MysqlConnPoolConfig& config)
{
	bool load_config = false;
	size_t i = 0;
	ConnectorInfo* ci = nullptr;
	for (i=0; i<config.write_conn_size; ++i) {
		ci = jmy_mem_malloc<ConnectorInfo>();
		write_connectors_.push_back(ci);
		if (!one_connector_init(ci->connector, config)) {
			LogError("write connector init failed");
			return false;
		}
		if (!load_config) {
			config_loader_.setConnector(&ci->connector);
			if (!config_loader_.load(*config.database_config)) {
				return false;
			}
			load_config = true;
		}
		threads_.create_thread(std::bind(connector_write_func, ci));
	}
	for (; i<config.read_conn_size; ++i) {
		ci = jmy_mem_malloc<ConnectorInfo>();
		read_connectors_.push_back(ci);
		if (!one_connector_init((ci->connector), config)) {
			LogError("read connector init failed");
			return false;
		}
		threads_.create_thread(std::bind(connector_read_func, ci));
	}
	return true;
}

void MysqlConnectorPool::close()
{
	threads_.join_all();

	size_t i = 0;
	for (; i<read_connectors_.size(); ++i) {
		ConnectorInfo* conn = read_connectors_[i];
		if (!conn) continue;
		conn->clear();
		jmy_mem_free(conn);
	}
	read_connectors_.clear();
	curr_read_index_ = 0;
	
	for (; i<write_connectors_.size(); ++i) {
		ConnectorInfo* conn = write_connectors_[i];
		if (!conn) continue;
		conn->clear();
		jmy_mem_free(conn);
	}
	write_connectors_.clear();
	curr_write_index_ = 0;
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

// call by main thread
int MysqlConnectorPool::run()
{
	ResultInfo ri;
	ConnectorInfo* ci = nullptr;
	size_t i = 0;
	size_t s = read_connectors_.size();
	for (; i<s; ++i) {
		ci = read_connectors_[i];
		if (ci->pop_res(ri)) {
			if (ri.cb_func) {
				ri.cb_func(ri.res, ri.user_param, ri.user_param_l);
			}
		}
	}
	s = write_connectors_.size();
	for (i=0; i<s; ++i) {
		ci = write_connectors_[i];
		if (ci->pop_res(ri)) {
			if (ri.cb_func) {
				ri.cb_func(ri.res, ri.user_param, ri.user_param_l);
			}
		}
	}
	return 0;
}
