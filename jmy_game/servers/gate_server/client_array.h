#pragma once

#include <string>
#include <list>
#include <set>
#include "../libjmy/jmy_tcp_connection.h"

enum { DEFAULT_CLIENT_START_ID = 100000 };

struct ClientInfo {
	bool used;
	int id;
	std::string account;
	JmyTcpConnection* conn;
	std::string enter_session;
	std::string reconn_session;

	ClientInfo() : used(false), id(0), conn(nullptr) {}
	int send(int msg_id, const char* data, unsigned short len) {
		if (!used) return -1;
		if (!conn) return -1;
		return conn->send(msg_id, data, len);
	}
	void close() {
		if (!used || !conn) return;
		conn->close();
		used = false;
	}
	void force_close() {
		if (!used || !conn) return;
		conn->force_close();
	}
	bool check_conn(JmyTcpConnection* conn2) {
		return conn == conn2;
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
