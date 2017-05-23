#pragma once

#include <list>
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
		TableRecord* r = nullptr;
		if (!records_.find_1(key, r)) {
			return nullptr;
		}
		return r;
	} 

	TableRecord* get_new(const Key& key) {
		TableRecord* t = jmy_mem_malloc<TableRecord>();
		records_.insert(key, t);
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
		TableRecord* r = nullptr;
		if (!records_.find_1(key, r)) {
			return false;
		}
		if (r) {
			jmy_mem_free(r);
		}
		records_.remove_1(key);
		return true;
	}

	bool remove(TableRecord* r) {
		if (!records_.find_2(r, tmp_key_)) {
			return false;
		}
		jmy_mem_free(r);
		records_.remove_2(r);
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

	bool commit_insert_request(const Key& key) {
		return commit_insert_request(key, nullptr, nullptr, 0);
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
				if (!t->insert_request(it->second.data.insert_data.cb, it->second.data.insert_data.param, it->second.data.insert_data.param_l)) {
					LogError("insert request failed");
				} else {
				}
			} else if (it->second.state == MYSQL_TABLE_RECORD_STATE_DELETE) {
				if (!t->delete_request()) {
					LogError("delete request failed");
				} else {
					if (!remove(t)) {
						LogError("remove record failed");
					}
				}
			} else if (it->second.state == MYSQL_TABLE_RECORD_STATE_UPDATE) {
				if (!t->update_request()) {
					LogError("update request failed");
				} else {
				}
			} else {
				LogWarn("unsupported table record state ", it->second.state);
			}
		}
		states_.clear();
	}

protected:
	std::string key_name_;
	BiMap<Key, TableRecord*> records_;
	std::unordered_map<TableRecord*, record_state_info> states_;
	Key tmp_key_;
};

template <typename TableRecord, typename Key, typename Key2>
class mysql_records_manager2 : public mysql_records_manager<TableRecord, Key>
{
public:
	mysql_records_manager2();
	~mysql_records_manager2();

	void init(const char* key_name, const char* key2_name) {
		mysql_records_manager<TableRecord, Key>::init(key_name);
		key2_name_ = key2_name;
	}

	bool insert_pair(const Key& key, const Key2& key2) {
		if (!key_map_.find_1(key, tmp_key2_))
			return false;
		key_map_.insert(key, key2);
		return true;
	}

	TableRecord* get_by_key(const Key& key) {
		return mysql_records_manager<TableRecord, Key>::get(key);
	}

	TableRecord* get_by_key2(const Key2& key2) {
		if (!key_map_.find_2(key2, tmp_key_))
			return nullptr;
		TableRecord* r = nullptr;
		if (!mysql_records_manager<TableRecord, Key>::records_.find_1(tmp_key_, r))
			return nullptr;
		return r;
	}

	TableRecord* get_new_by_key(const Key& key) {
		return mysql_records_manager<TableRecord, Key>::get_new(key);
	}

	TableRecord* get_new_by_key2(const Key2& key2) {
		if (!key_map_.find_2(key2, tmp_key_))
			return nullptr;
		return mysql_records_manager<TableRecord, Key>::get_new(tmp_key_);
	}

	bool remove_by_key(const Key& key) {
		if (!key_map_.remove_1(key))
			return false;
		return mysql_records_manager<TableRecord, Key>::remove(key);
	}

	bool remove_by_key2(const Key2& key2) {
		if (!key_map_.find_2(key2, tmp_key_))
			return false;
		key_map_.remove_2(key2);
		return mysql_records_manager<TableRecord, Key>::remove(tmp_key_);
	}

protected:
	BiMap<Key, Key2> key_map_;
	Key tmp_key_;
	Key2 tmp_key2_;
	std::string key2_name_;
};

template <typename TableRecord, typename Key>
class mysql_record_list
{
public:
	mysql_record_list() {}
	~mysql_record_list() { clear(); }

	void clear() {
		typename std::unordered_map<Key, TableRecord*>::iterator it = records_.begin();	
		for (; it!=records_.end(); ++it) {
			if (it->second) {
				jmy_mem_free(it->second);
			}
		}
		records_.clear();
	}

	TableRecord* get_new(const Key& key) {
		TableRecord* t = jmy_mem_malloc<TableRecord>();
		records_.insert(std::make_pair(key, t));	
		return t;
	}

	TableRecord* get(const Key& key) {
		typename std::unordered_map<Key, TableRecord*>::iterator it = records_.find(key);
		if (it == records_.end())
			return nullptr;
		return it->second;
	}

	bool remove(const Key& key) {
		typename std::unordered_map<Key, TableRecord*>::iterator it = records_.find(key);
		if (it == records_.end())
			return false;
		if (it->second) {
			jmy_mem_free(it->second);
		}
		records_.erase(it);
		return true;
	}

private:
	std::unordered_map<Key, TableRecord*> records_;
};

template <typename TableRecord, typename Key, typename SubKey>
class mysql_records_subkey_manager
{
public:
	mysql_records_subkey_manager() {}
	~mysql_records_subkey_manager() {}

	typedef mysql_record_list<TableRecord, SubKey> ValueType;

	ValueType* get(const Key& key) {
		typename std::unordered_map<Key, ValueType*>::iterator it = key2recordlist_.find(key);
		if (it == key2recordlist_.end())
			return nullptr;
		return it->second;
	}

	ValueType* get_new(const Key& key) {
		ValueType* v = jmy_mem_malloc<ValueType>();
		key2recordlist_.insert(std::make_pair(key, v));
		return v;
	}

	bool remove(const Key& key) {
		typename std::unordered_map<Key, ValueType*>::iterator it = key2recordlist_.find(key);
		if (it == key2recordlist_.end())
			return false;
		if (it->second) {
			it->second->clear();
			jmy_mem_free(it->second);
		}
		key2recordlist_.erase(it);
		return true;
	}

	TableRecord* get_record(const Key& key, const SubKey& sub_key) {
		ValueType* v = get(key);
		if (!v) return nullptr;
		TableRecord* r = v->get(sub_key);	
		return r;
	}

	TableRecord* get_new_record(const Key& key, const SubKey& sub_key) {
		ValueType* v = get_new(key);
		if (!v) return nullptr;
		TableRecord* r = v->get_new(sub_key);
		record2key_.insert(r, std::make_pair(key, sub_key));
		return r;
	}

	bool remove_record(const Key& key, const SubKey& sub_key) {
		ValueType* v = get(key);
		if (!v) return false;
		TableRecord* r = v->get(sub_key);
		if (!r) return false;
		v->remove(sub_key);
		record2key_.erase(r);
		return true;
	}

	bool remove_record(TableRecord* record) {
		std::unordered_map<TableRecord*, std::pair<Key, SubKey> >::iterator it = record2key_.find(record);
		if (it == record2key_.end())
			return false;
		std::unordered_map<Key, ValueType*>::iterator kit = key2recordlist_.find(it->second.first);
		if (kit == key2recordlist_.end())
			return false;
		kit->remove(it->second.second);
		return true;
	}

protected:
	std::unordered_map<TableRecord*, std::pair<Key, SubKey> > record2key_;
	std::unordered_map<Key, ValueType*> key2recordlist_;
	std::unordered_map<TableRecord*, record_state_info> states_;
};
