#pragma once

#include "../libjmy/jmy_singleton.hpp"
#include "../libjmy/jmy_tcp_connection.h"

class GlobalData : public JmySingleton<GlobalData>
{
public:
	GlobalData() : gate_conn_(nullptr), db_conn_(nullptr) {}
	GlobalData(JmyTcpConnection* gate_conn, JmyTcpConnection* db_conn) : gate_conn_(gate_conn), db_conn_(db_conn) {}
	~GlobalData() {}

	void setGateConn(JmyTcpConnection* conn) { gate_conn_ = conn; }
	void setDBConn(JmyTcpConnection* conn) { db_conn_ = conn; }

	int sendGate(int msg_id, const char* data, unsigned short len) {
		if (!gate_conn_) return -1;
		return gate_conn_->send(msg_id, data, len);
	}
	int sendGate(int user_id, int msg_id, const char* data, unsigned short len) {
		if (!gate_conn_) return -1;
		return gate_conn_->send(user_id, msg_id, data, len);
	}
	int sendDB(int msg_id, const char* data, unsigned short len) {
		if (!db_conn_) return -1;
		return db_conn_->send(msg_id, data, len);
	}
	int sendDB(int user_id, int msg_id, const char* data, unsigned short len) {
		if (!db_conn_) return -1;
		return db_conn_->send(user_id, msg_id, data, len);
	}

private:
	JmyTcpConnection* gate_conn_;
	JmyTcpConnection* db_conn_;
};

#define GLOBAL_DATA (GlobalData::getInstance())

#define SEND_GATE_MSG(msg_id, data, len)				(GlobalData::getInstance()->sendGate(msg_id, data, len))
#define SEND_GATE_USER_MSG(user_id, msg_id, data, len)	(GlobalData::getInstance()->sendGate(user_id, msg_id, data, len))
#define SEND_DB_MSG(msg_id, data, len)					(GlobalData::getInstance()->sendDB(msg_id, data, len))
#define SEND_DB_USER_MSG(user_id, msg_id, data, len)	(GlobalData::getInstance()->sendDB(user_id, msg_id, data, len))
