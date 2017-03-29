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

template <typename data_type, typename int_type>
class Agent
{
public:
	Agent() : data_(), id_(0), mgr_(nullptr), state_(AGENT_STATE_NONE) {}
	~Agent() {}
	void init(int id, JmyTcpConnectionMgr* mgr) {
		id_ = id;
		mgr_ = mgr;
		state_ = AGENT_STATE_NONE;
	}

	int_type getId() const { return id_; }
	AgentStateType getState() const { return state_; }
	void setState(AgentStateType state) { state_ = state; }
	data_type& getData() const { return data_; }
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
			ServerLogError("agent(%d) send message failed", id_);
			return -1;
		}

		return 0;
	}

private:
	data_type data_;
	int_type id_;
	JmyTcpConnectionMgr* mgr_;
	AgentStateType state_;
};

template <typename data_type, typename int_type>
class AgentManager 
{
public:
	typedef Agent<data_type, int_type> agent_type;
	typedef std::unordered_map<int_type, agent_type*> agents_map;

	AgentManager() {}
	~AgentManager() { clear(); }

	void clear() {
		typename agents_map::iterator it = id2agents_.begin();
		for (; it!=id2agents_.end(); ++it) {
			if (it->second) {
				jmy_mem_free(it->second);
			}
		}
		id2agents_.clear();
	}

	agent_type* newAgent(int_type id, JmyTcpConnectionMgr* mgr) {
		typename agents_map::iterator it = id2agents_.find(id);
		if (it != id2agents_.end()) {
			ServerLogError("already has user(%d), create user failed", id);
			return nullptr;
		}
		agent_type* agent = jmy_mem_malloc<agent_type>();
		agent->init(id, mgr);
		id2agents_.insert(std::make_pair(id, agent));
		return agent;
	}

	agent_type* getAgent(int_type id) {
		typename agents_map::iterator it = id2agents_.find(id);
		if (it == id2agents_.end()) {
			ServerLogError("cant found account(%d)", id);
			return nullptr;
		}
		return it->second;
	}

	bool deleteAgent(int_type id) {
		typename agents_map::iterator it = id2agents_.find(id);
		if (it == id2agents_.end()) {
			ServerLogError("cant found id(%d)", id);
			return false;
		}
		jmy_mem_free(it->second);
		id2agents_.erase(it);
		return true;
	}

	bool setAgentState(int_type id, AgentStateType state) {
		typename agents_map::iterator it = id2agents_.find(id);
		if (it == id2agents_.end()) {
			ServerLogError("not found user(%d)", id);
			return false;
		}
		it->second->setState(state);
		return true;
	}

protected:
	agents_map id2agents_;
};

template <typename data_type, typename int_type, size_t max_id>
class AgentManagerPerf
{
public:
	typedef Agent<data_type, int_type> agent_type;
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

	agent_type* newAgent(int_type id, JmyTcpConnectionMgr* mgr) {
		agent_type* agent = getAgent(id);
		if (!agent) {
			ServerLogError("already has agent with id(%d)", id);
			return nullptr;
		}
		agent = jmy_mem_malloc<agent_type>();
		agent->init(id, mgr);
		agents_[id] = agent;
		agent_count_ += 1;
		return agent;
	}

	agent_type* getAgent(int_type id) {
		if (id > agents_.size()-1) {
			ServerLogError("id(%d) overed array size(%u)", id, agents_.size());
			return nullptr;
		}
		return agents_.at(id);
	}

	bool deleteAgent(int_type id) {
		agent_type* agent = getAgent(id);
		if (!agent) {
			return false;
		}
		jmy_mem_free<agent_type>(agent);
		if (agent_count_ == 0) {
			ServerLogError("agent count is zero, cant delete agent(%d)", id);
			return false;
		}
		agents_[id] = nullptr;
		agent_count_ -= 1;
		return true;
	}

	bool setAgentState(int_type id, AgentStateType state) {
		agent_type* agent = getAgent(id);
		if (!agent)  {
			ServerLogError("not found agent with id(%d)", id);
			return false;
		}
		agent->setState(state);
	}

protected:
	agent_array_type agents_;
	size_t agent_count_;
};
