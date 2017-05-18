#pragma once

#include <string>
#include <list>
#include <set>
#include <vector>
#include "../libjmy/jmy_tcp_connection.h"
#include "../../proto/src/common.pb.h"

enum { DEFAULT_CLIENT_START_ID = 100000 };

struct ClientInfo {
	bool used;
	int id;
	uint64_t curr_uid;
	std::string account;
	JmyTcpConnection* conn;
	std::string enter_session;
	std::string reconn_session;
	bool get_role_list;
	std::vector<MsgBaseRoleData*> role_list;

	ClientInfo() : used(false), id(0), curr_uid(0), conn(nullptr), get_role_list(false) {}
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

	bool has_role(uint64_t role_id) {
		std::vector<MsgBaseRoleData*>::iterator it = role_list.begin();
		for (; it!=role_list.end(); ++it) {
			MsgBaseRoleData* p = *it;
			if (p && p->role_id() == role_id)
				return true;
		}
		return false;
	}

	bool add_role(const MsgBaseRoleData& role) {
		if (has_role(role.role_id()))
			return false;

		MsgBaseRoleData* r = new MsgBaseRoleData;
		*r = role;
		role_list.push_back(r);
		return true;
	}

	bool remove_role(uint64_t role_id) {
		std::vector<MsgBaseRoleData*>::iterator it = role_list.begin();
		for (; it!=role_list.end(); ++it) {
			MsgBaseRoleData* p = *it;
			if (p && p->role_id() == role_id) {
				role_list.erase(it);
				return true;
			}
		}
		return false;
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
