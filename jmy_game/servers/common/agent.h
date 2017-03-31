#pragma once

#include <unordered_map>
#include <array>
#include <string>
#include "../libjmy/jmy_tcp_connection.h"
#include "../libjmy/jmy_mem.h"
#include "util.h"

enum AgentStateType {
	AGENT_STATE_NONE = 0,
	AGENT_STATE_CONNECTED = 1,
	AGENT_STATE_VERIFIED = 2,
	AGENT_STATE_DISCONNECT = 3,
};

class JmyTcpConnectionMgr;

template <typename data_type, typename id_type>
class Agent
{
public:
	typedef typename std::enable_if<std::is_integral<id_type>::value, id_type>::type int_id_type;

	Agent() : data_(), id_(0), mgr_(nullptr), state_(AGENT_STATE_NONE) {}
	~Agent() {}
	void init(int_id_type id, JmyTcpConnectionMgr* mgr) {
		id_ = id;
		mgr_ = mgr;
		state_ = AGENT_STATE_NONE;
	}

	int_id_type getId() const { return id_; }
	AgentStateType getState() const { return state_; }
	void setState(AgentStateType state) { state_ = state; }
	data_type& getData() { return data_; }
	data_type* getDataPointer() { return &data_; }
	void setData(const data_type& data) { data_ = data; }
	void setData(data_type&& data) { data_ = data; }

	int sendMsg(int msgid, const char* data, int len) {
		if (state_ != AGENT_STATE_VERIFIED) return 0;
		JmyTcpConnection* conn = mgr_->get(id_);
		if (!conn) {
			ServerLogError("not found connection with id(%d)", id_);
			return -1;
		}

		if (conn->send(msgid, data, len) < 0) {
			ServerLogError("agent(%d) send message(%d) failed", id_, msgid);
			return -1;
		}

		return 0;
	}

	int close() {
		JmyTcpConnection* conn = mgr_->get(id_);
		if (!conn) return -1;
		conn->close();
		return 0;
	}

private:
	data_type data_;
	int_id_type id_;
	JmyTcpConnectionMgr* mgr_;
	AgentStateType state_;
};

template <typename key_type, typename data_type, typename id_type>
class AgentManager 
{
public:
	typedef Agent<data_type, id_type> agent_type;
	typedef typename agent_type::int_id_type int_id_type;
	typedef std::unordered_map<key_type, agent_type*> key2agent_map_type;
	typedef std::unordered_map<int_id_type, key_type> id2key_map_type;
	typedef std::unordered_map<key_type, int_id_type> key2id_map_type;

	AgentManager() {}
	~AgentManager() { clear(); }

	void clear() {
		typename key2agent_map_type::iterator it = key2agents_.begin();
		for (; it!=key2agents_.end(); ++it) {
			if (it->second) {
				jmy_mem_free(it->second);
			}
		}
		key2agents_.clear();
		id2keys_.clear();
	}

	agent_type* newAgent(const key_type& key, JmyTcpConnectionMgr* mgr, int_id_type id) {
		typename key2agent_map_type::iterator it = key2agents_.find(key);
		if (it != key2agents_.end()) { return nullptr; }
		agent_type* agent = jmy_mem_malloc<agent_type>();
		agent->init(id, mgr);
		key2agents_.insert(std::make_pair(key, agent));
		id2keys_.insert(std::make_pair(id, key));
		key2ids_.insert(std::make_pair(key, id));
		return agent;
	}

	agent_type* getAgent(key_type key) {
		typename key2agent_map_type::iterator it = key2agents_.find(key);
		if (it == key2agents_.end()) { return nullptr; }
		return it->second;
	}

	agent_type* getAgentById(int_id_type id) {
		typename id2key_map_type::iterator it = id2keys_.find(id);
		if (it == id2keys_.end()) { return nullptr; }
		return getAgent(it->second);
	}

	bool deleteAgent(key_type key) {
		typename key2agent_map_type::iterator it = key2agents_.find(key);
		if (it == key2agents_.end()) { return false; }
		typename key2id_map_type::iterator kit = key2ids_.find(key);
		if (kit != key2ids_.end()) {
			id2keys_.erase(kit->second);
			key2ids_.erase(kit);
		}
		jmy_mem_free(it->second);
		key2agents_.erase(it);
		return true;
	}

	bool deleteAgentById(int_id_type id) {
		typename id2key_map_type::iterator it = id2keys_.find(id);
		if (it == id2keys_.end()) { return false; }
		if (!deleteAgent(it->second)) { return false; }
		return true;
	}

	bool setAgentState(key_type key, AgentStateType state) {
		typename key2agent_map_type::iterator it = key2agents_.find(key);
		if (it == key2agents_.end()) { return false; }
		if (!it->second) { return false; }
		it->second->setState(state);
		return true;
	}

	bool setAgentStateById(int_id_type id, AgentStateType state) {
		typename id2key_map_type::iterator it = id2keys_.find(id);
		if (it == id2keys_.end()) { return false; }
		return setAgentState(it->second, state);
	}

protected:
	key2agent_map_type key2agents_;
	id2key_map_type id2keys_;
	key2id_map_type key2ids_;
};

template <typename data_type, typename id_type, size_t max_id>
class AgentManagerPerf
{
public:
	typedef Agent<data_type, id_type> agent_type;
	typedef typename agent_type::int_id_type int_id_type;
	typedef std::array<agent_type*, max_id+1> agent_array_type;
	AgentManagerPerf() : agent_count_(0) { init(); }
	~AgentManagerPerf() { clear(); }

	void init() {
		typename agent_array_type::iterator it = agents_.begin();
		for (; it!=agents_.end(); ++it) {
			*it = nullptr;
		}
	}

	void clear() {
		typename agent_array_type::iterator it = agents_.begin();
		for (; it!=agents_.end(); ++it) {
			agent_type* agent = *it;
			if (agent) {
				jmy_mem_free(agent);
				*it = nullptr;
			}
		}
	}

	agent_type* newAgent(int_id_type id, JmyTcpConnectionMgr* mgr) {
		agent_type* agent = getAgent(id);
		if (!agent) {
			return nullptr;
		}
		agent = jmy_mem_malloc<agent_type>();
		agent->init(id, mgr);
		agents_[id] = agent;
		agent_count_ += 1;
		return agent;
	}

	agent_type* getAgent(int_id_type id) {
		if (id > agents_.size()-1) {
			return nullptr;
		}
		return agents_.at(id);
	}

	bool deleteAgent(int_id_type id) {
		agent_type* agent = getAgent(id);
		if (!agent) {
			return false;
		}
		jmy_mem_free<agent_type>(agent);
		if (agent_count_ == 0) {
			return false;
		}
		agents_[id] = nullptr;
		agent_count_ -= 1;
		return true;
	}

	bool setAgentState(int_id_type id, AgentStateType state) {
		agent_type* agent = getAgent(id);
		if (!agent)  {
			return false;
		}
		agent->setState(state);
	}

protected:
	agent_array_type agents_;
	size_t agent_count_;
};
