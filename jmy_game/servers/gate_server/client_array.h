#pragma once

#include <string>
#include <list>
#include <set>
#include <vector>
#include "../libjmy/jmy_tcp_connection.h"
#include "../common/util.h"
#include "../../proto/src/common.pb.h"

enum { DEFAULT_CLIENT_START_ID = 100000 };

class JmyTcpConnectionMgr;
struct ClientInfo {
	bool used;
	int id;
	uint64_t curr_role_id;
	std::string account;
	int conn_id;
	JmyTcpConnectionMgr* conn_mgr;
	std::string enter_session;
	std::string reconn_session;
	std::vector<uint64_t> role_id_list;

	ClientInfo() : used(false), id(0), curr_role_id(0), conn_id(0), conn_mgr(nullptr) {}
	int send(int msg_id, const char* data, unsigned short len) {
		if (!used) return -1;
		JmyTcpConnection* conn = get_connection(conn_id, conn_mgr);
		if (!conn) return -1;
		return conn->send(msg_id, data, len);
	}
	void close() {
		if (!used) return;
		JmyTcpConnection* conn = get_connection(conn_id, conn_mgr);
		if (!conn) return;
		conn->close();
		used = false;
	}
	void force_close() {
		if (!used) return;
		JmyTcpConnection* conn = get_connection(conn_id, conn_mgr);
		if (!conn) return;
		conn->force_close();
	}
	bool check_conn(JmyTcpConnection* conn2) {
		JmyTcpConnection* conn = get_connection(conn_id, conn_mgr);
		if (!conn)
			return false;
		return conn == conn2;
	}
	bool has_role_id(uint64_t role_id) {
		std::vector<uint64_t>::iterator it = role_id_list.begin();
		for (; it!=role_id_list.end(); ++it) {
			if (*it == role_id)
				return true;
		}
		return false;
	}
	void add_role_id(uint64_t role_id) {
		if (!has_role_id(role_id))
			role_id_list.push_back(role_id);
	}
};

class ClientArray
{
public:
	ClientArray();
	~ClientArray();

	bool init(int array_size, int start_id = DEFAULT_CLIENT_START_ID);
	void clear();
	void reset();
	ClientInfo* getFree();
	ClientInfo* get(int id);
	bool free(ClientInfo* info);
	bool free(int id);
	int getUsedSize() const { return used_ids_.size(); }
	int getStartId() const { return start_id_; }

private:
	ClientInfo** info_array_;
	int start_id_;
	int info_size_;
	std::list<int> free_ids_;
	std::set<int> used_ids_;
	bool inited_;
};
