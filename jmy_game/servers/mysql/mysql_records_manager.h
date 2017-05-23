#pragma once

#include <unordered_map>
#include "mysql_defines.h"
#include "mysql_connector.h"
#include "../libjmy/jmy_mem.h"
#include "../common/util.h"
#include "../common/bi_map.h"

struct record_state_info {
	MysqlTableRecordState state;
	union union_data {
		struct insert_info {
			mysql_cmd_callback_func cb;
		  	void* param;
			long l_param;
		} insert_data;
		
		struct delete_info {
			const char* key;
		} delete_data;
		
		struct update_info {
		} update_data;
	} data;

	record_state_info() : state(MYSQL_TABLE_RECORD_STATE_NONE) {}
};

template <typename TableRecord, typename Key>
class mysql_records_manager
{
public:
	mysql_records_manager() {}
	~mysql_records_manager() {}
	
	void init(const char* key_name) {
		key_name_ = key_name;
	}

	TableRecord* get(const Key& key) {
		typename std::unordered_map<Key, TableRecord*>::iterator it = records_.find(key);
		if (it == records_.end()) {
			return nullptr;
		}
		return it->second;
	} 

	TableRecord* get_new(const Key& key) {
		TableRecord* t = jmy_mem_malloc<TableRecord>();
		records_.insert(std::make_pair(key, t));
		return t;
	}

	record_state_info* get_state(TableRecord* t) {
		typename std::unordered_map<TableRecord*, record_state_info>::iterator it = states_.find(t);
		if (it == states_.end()) {
			return nullptr;
		}
		return &it->second;
	}

	bool remove(const Key& key) {
		typename std::unordered_map<Key, TableRecord*>::iterator it = records_.find(key);
		if (it == records_.end()) {
			return false;
		}
		if (it->second) {
			jmy_mem_free(it->second);
		}
		records_.erase(it);
		return true;
	}

	bool commit_insert_request(const Key& key, mysql_cmd_callback_func cb, void* param, long param_l) {
		TableRecord* t = get(key);
		if (t) {
			record_state_info* s = get_state(t);
			if (s->state == MYSQL_TABLE_RECORD_STATE_NONE) {
				s->state = MYSQL_TABLE_RECORD_STATE_INSERT;
			} else if (s->state == MYSQL_TABLE_RECORD_STATE_DELETE) {
				s->state = MYSQL_TABLE_RECORD_STATE_NONE;
			} else if (s->state == MYSQL_TABLE_RECORD_STATE_INSERT) {
				// do nothing
			} else if (s->state == MYSQL_TABLE_RECORD_STATE_UPDATE) {
				// do nothing
				LogWarn("to update record not allow to insert request");
			} else {
				LogError("unknown table record state ", s->state);
				return false;
			}
		} else {
			t = get_new(key);
			record_state_info s;
			s.state = MYSQL_TABLE_RECORD_STATE_INSERT;
			s.data.insert_data.cb = cb;
			s.data.insert_data.param = param;
			s.data.insert_data.l_param = param_l;
			states_.insert(std::make_pair(t, s));
		}
		return true;
	}

	bool commit_delete_request(const Key& key_value) {
		TableRecord* t = get(key_value);
		if (!t) {
			LogInfo("no record to request delete");
			return false;
		}

		record_state_info* s = get_state(t);
		if (!s) {
			record_state_info ss;
			ss.state = MYSQL_TABLE_RECORD_STATE_DELETE;
			ss.data.delete_data.key = key_name_;
			states_.insert(std::make_pair(t, ss));
		} else {
			if (s->state == MYSQL_TABLE_RECORD_STATE_NONE) {
				s->state = MYSQL_TABLE_RECORD_STATE_DELETE;
			} else if (s->state == MYSQL_TABLE_RECORD_STATE_INSERT) {
				s->state = MYSQL_TABLE_RECORD_STATE_NONE;
			} else if (s->state == MYSQL_TABLE_RECORD_STATE_DELETE) {
				// do nothing
			} else if (s->state == MYSQL_TABLE_RECORD_STATE_UPDATE) {
				s->state = MYSQL_TABLE_RECORD_STATE_DELETE;
			} else {
				LogError("unsupported record state(%d) to request delete", s->state);
				return false;
			}
		}

		return true;
	}

	bool commit_update_request(const Key& key_value) {
		TableRecord* t = get(key_value);
		if (!t) {
			LogError("not found record");
			return false;
		}
		record_state_info* s = get_state(t);
		if (!s) {
			record_state_info ss;
			ss.state = MYSQL_TABLE_RECORD_STATE_UPDATE;
			states_.insert(std::make_pair(t, ss));
		} else {
			if (s->state == MYSQL_TABLE_RECORD_STATE_NONE) {
				s->state = MYSQL_TABLE_RECORD_STATE_UPDATE;
			} else if (s->state == MYSQL_TABLE_RECORD_STATE_INSERT) {
				// do nothing
			} else if (s->state == MYSQL_TABLE_RECORD_STATE_DELETE) {
				LogWarn("to delete record not allow to update request");
			} else if (s->state == MYSQL_TABLE_RECORD_STATE_UPDATE) {
				// do nothing
			} else {
				LogError("unknown state(%d) on update request", s->state);
				return false;
			}
		}

		return true;
	}

	void update() {
		typename std::unordered_map<TableRecord*, record_state_info>::iterator it = states_.begin();
		for (; it!=states_.end(); ++it) {
			TableRecord* t = it->first;
			if (!t) {
				LogWarn("null record");
				continue;
			}
			if (it->second.state == MYSQL_TABLE_RECORD_STATE_INSERT) {
				t->insert_request();
			} else if (it->second.state == MYSQL_TABLE_RECORD_STATE_DELETE) {
				t->delete_request(key_name_);
			} else if (it->second.state == MYSQL_TABLE_RECORD_STATE_UPDATE) {
				t->update_request(key_name_);
			} else {
				LogWarn("unsupported table record state ", it->second.state);
			}
		}
		states_.clear();
	}

protected:
	std::string key_name_;
	std::unordered_map<Key, TableRecord*> records_;
	std::unordered_map<TableRecord*, record_state_info> states_;
};

template <typename Table, typename Key, typename Key2>
class mysql_records_manager2 : public mysql_records_manager<Table, Key>
{
public:
	mysql_records_manager2();
	~mysql_records_manager2();

	void init(const char* key_name, const char* key2_name) {
		mysql_records_manager<Table, Key>::init(key_name);
		key2_name_ = key2_name;
	}

	bool insert_pair(const Key& key, const Key2& key2) {
		if (!key_map_.find_1(key, tmp_key2_))
			return false;
		key_map_.insert(key, key2);
		return true;
	}

	bool remove_by_key(const Key& key) {
		if (!key_map_.remove_1(key))
			return false;
		return mysql_records_manager<Table, Key>::remove(key);
	}

	bool remove_by_key2(const Key2& key2) {
		if (!key_map_.find_2(key2, tmp_key_))
			return false;
		key_map_.remove_2(key2);
		return mysql_records_manager<Table, Key>::remove(tmp_key_);
	}

protected:
	BiMap<Key, Key2> key_map_;
	Key tmp_key_;
	Key2 tmp_key2_;
	std::string key2_name_;
};
