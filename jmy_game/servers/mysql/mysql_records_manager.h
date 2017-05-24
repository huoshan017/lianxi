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

template <typename TableRecord>
class mysql_record_state_manager
{
public:
	mysql_record_state_manager();
	~mysql_record_state_manager();

	void clear() {
		states_.clear();
	}

	record_state_info* get_state(TableRecord* t) {
		typename std::unordered_map<TableRecord*, record_state_info>::iterator it = states_.find(t);
		if (it == states_.end()) {
			return nullptr;
		}
		return &it->second;
	}

	bool commit_insert(TableRecord* record, mysql_cmd_callback_func cb, void* param, long param_l) {
		record_state_info* s = get_state(record);
		if (s) {
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
			record_state_info state;
			state.state = MYSQL_TABLE_RECORD_STATE_INSERT;
			state.data.insert_data.cb = cb;
			state.data.insert_data.param = param;
			state.data.insert_data.l_param = param_l;
			states_.insert(std::make_pair(record, s));
		}
		return true;
	}

	bool commit_delete(TableRecord* record) {
		record_state_info* s = get_state(record);
		if (!s) {
			record_state_info ss;
			ss.state = MYSQL_TABLE_RECORD_STATE_DELETE;
			states_.insert(std::make_pair(record, ss));
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

	bool commit_update(TableRecord* record) {
		record_state_info* s = get_state(record);
		if (!s) {
			record_state_info ss;
			ss.state = MYSQL_TABLE_RECORD_STATE_UPDATE;
			states_.insert(std::make_pair(record, ss));
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

	void start_update() {
		curr_iter_ = states_.begin();
	}

	bool next_update_record() {
		if (curr_iter_ == states_.end())
			return false;
		++curr_iter_;
		return true;
	}

	TableRecord* get_update_record() {
		if (curr_iter_ == states_.end())
			return nullptr;
		TableRecord* record = curr_iter_->first;
		++curr_iter_;
		return record;
	}

	int update_one_record(TableRecord* record) {
		if (curr_iter_->second.state == MYSQL_TABLE_RECORD_STATE_INSERT) {
			if (!record->insert_request(
						curr_iter_->second.data.insert_data.cb,
						curr_iter_->second.data.insert_data.param,
						curr_iter_->second.data.insert_data.l_param)) {
				LogError("insert request failed");
				return 0;
			}
		} else if (curr_iter_->second.state == MYSQL_TABLE_RECORD_STATE_DELETE) {
			if (!record->delete_request()) {
				LogError("delete request failed");
				return 0;
			} else {
				return 2;
			}
		} else if (curr_iter_->second.state == MYSQL_TABLE_RECORD_STATE_UPDATE) {
			if (!record->update_request()) {
				LogError("update request failed");
				return 0;
			}
		} else {
			LogWarn("unsupported table record state ", curr_iter_->second.state);
		}
		return 1;
	}

private:
	std::unordered_map<TableRecord*, record_state_info> states_;
	typename std::unordered_map<TableRecord*, record_state_info>::iterator curr_iter_;
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
		TableRecord* record = get(key);
		if (!record) record = get_new(key);
		return state_mgr_.commit_insert(record, cb, param, param_l);
	}

	bool commit_insert_request(TableRecord* record, mysql_cmd_callback_func cb, void* param, long param_l) {
		return state_mgr_.commit_insert(record, cb, param, param_l);
	}

	bool commit_delete_request(const Key& key_value) {
		TableRecord* record = get(key_value);
		if (!record) {
			LogInfo("no record to request delete");
			return false;
		}
		return state_mgr_.commit_delete(record);
	}

	bool commit_delete_request(TableRecord* record) {
		return state_mgr_.commit_delete(record);
	}

	bool commit_update_request(const Key& key_value) {
		TableRecord* record = get(key_value);
		if (!record) {
			LogError("not found record");
			return false;
		}
		return state_mgr_.commit_update(record);
	}

	bool commit_update_request(TableRecord* record) {
		return state_mgr_.commit_update(record);
	}

	void update() {
		state_mgr_.start_update();
		TableRecord* record = nullptr;
		while ((record = state_mgr_.get_update_record()) != nullptr) {
			int res = state_mgr_.update_one_record(record);
			if (res < 0) {
				return;
			} else if (res == 2) {
				remove(record);
			}
		}
		state_mgr_.clear();
	}

protected:
	BiMap<Key, TableRecord*> records_;
	mysql_record_state_manager<TableRecord> state_mgr_;
	std::string key_name_;
	Key tmp_key_;
};

template <typename TableRecord, typename Key, typename Key2>
class mysql_records_manager2
{
public:
	mysql_records_manager2();
	~mysql_records_manager2();

	void init(const char* key_name, const char* key2_name) {
		key_name_ = key_name;
		key2_name_ = key2_name;
	}

	bool make_pair(const Key& key, const Key2& key2) {
		TableRecord* record = nullptr;
		if (!key_record_map_.find_1(key, record)) {
			if (!key2_record_map_.find_1(key2, record))
				return false;
			key2_record_map_.insert(key2, record);
		} else {
			key_record_map_.insert(key, record);
		}
		return true;
	}

	TableRecord* get_by_key(const Key& key) {
		TableRecord* record = nullptr;
		if (!key_record_map_.find_1(key, record))
			return nullptr;
		return record;
	}

	TableRecord* get_by_key2(const Key2& key2) {
		TableRecord* record = nullptr;
		if (!key2_record_map_.find_1(key2, record))
			return nullptr;
		return record;
	}

	TableRecord* get_new_by_key(const Key& key) {
		TableRecord* record = jmy_mem_malloc<TableRecord>();
		key_record_map_.insert(key, record);
		return record;
	}

	TableRecord* get_new_by_key2(const Key2& key2) {
		TableRecord* record = jmy_mem_malloc<TableRecord>();
		key2_record_map_.insert(key2, record);
		return record;
	}

	bool remove_by_key(const Key& key) {
		TableRecord* record = nullptr;
		if (!key_record_map_.find_1(key, record))
			return false;
		if (!key2_record_map_.remove_2(record))
			return false;
		key_record_map_.remove_1(key);
		jmy_mem_free(record);
		return true;
	}

	bool remove_by_key2(const Key2& key2) {
		TableRecord* record = nullptr;
		if (!key2_record_map_.find_1(key2, record))
			return false;
		if (!key_record_map_.remove_2(record))
			return false;
		key2_record_map_.remove_1(key2);
		jmy_mem_free(record);
		return true;
	}

	bool remove_record(TableRecord* record) {
		if (!key_record_map_.find_2(record, tmp_key_)) {
			LogWarn("cant find key by record");
			return false;
		}
		key_record_map_.remove_2(record);
		if (!key2_record_map_.find_2(record, tmp_key2_)) {
			LogWarn("cant find key2 by record");
			return false;
		}
		jmy_mem_free(record);
		return true;
	}

	bool commit_insert_request_by_key(const Key& key, mysql_cmd_callback_func cb, void* param, long param_l) {
		TableRecord* record = get_by_key(key);
		if (!record) record = get_new_by_key2(key);
		return state_mgr_.commit_insert(record, cb, param, param_l);
	}

	bool commit_insert_request_by_key2(const Key2& key2, mysql_cmd_callback_func cb, void* param, long param_l) {
		TableRecord* record = get_by_key2(key2);
		if (!record) record = get_new_by_key2(key2);
		return state_mgr_.commit_insert(record, cb, param, param_l);
	}

	bool commit_insert_request(TableRecord* record, mysql_cmd_callback_func cb, void* param, long param_l) {
		return state_mgr_.commit_insert(record, cb, param, param_l);
	}

	bool commit_delete_request_by_key(const Key& key) {
		TableRecord* record = get_by_key(key);
		if (!record) {
			LogInfo("no record to request delete by key");
			return false;
		}
		return state_mgr_.commit_delete(record);
	}

	bool commit_delete_request_by_key2(const Key2& key2) {
		TableRecord* record = get_by_key2(key2);
		if (!record) {
			LogError("no record to request delete by key2");
			return false;
		}
		return state_mgr_.commit_delete(record);
	}

	bool commit_delete_request(TableRecord* record) {
		return state_mgr_.commit_delete(record);
	}

	bool commit_update_request_by_key(const Key& key) {
		TableRecord* record = get_by_key(key);
		if (!record) {
			LogError("no record to request update by key");
			return false;
		}
		return state_mgr_.commit_update(record);
	}

	bool commit_update_request_by_key2(const Key2& key2) {
		TableRecord* record = get_by_key2(key2);
		if (!record) {
			LogError("no record to request update by key2");
			return false;
		}
		return state_mgr_.commit_update(record);
	}

	bool commit_update_request(TableRecord* record) {
		return state_mgr_.commit_update(record);
	}

	void update() {
		state_mgr_.start_update();
		TableRecord* record = nullptr;
		while ((record = state_mgr_.get_update_record()) != nullptr) {
			int res = state_mgr_.update_one_record(record);
			if (res < 0) {
				return;
			} else if (res == 2) {
				remove_record(record);
			}
		}
		state_mgr_.clear();
	}

protected:
	BiMap<Key, TableRecord*> key_record_map_;
	BiMap<Key2, TableRecord*> key2_record_map_;
	mysql_record_state_manager<TableRecord> state_mgr_;
	std::string key_name_;
	std::string key2_name_;
	Key2 tmp_key2_;
	Key tmp_key_;
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

	TableRecord* get_new_no_insert() {
		return jmy_mem_malloc<TableRecord>();
	}

	bool insert_new(const Key& key, TableRecord* record) {
		if (records_.find(key) == records_.end())
			return false;
		records_.insert(std::make_pair(key, record));
		return true;
	}

	TableRecord* get_new(const Key& key) {
		TableRecord* record = jmy_mem_malloc<TableRecord>();
		records_.insert(std::make_pair(key, record));	
		return record;
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

	bool remove(const Key& key, const SubKey& sub_key) {
		ValueType* v = get(key);
		if (!v) return false;
		TableRecord* r = v->get(sub_key);
		if (!r) return false;
		v->remove(sub_key);
		record2key_.erase(r);
		return true;
	}

	bool remove_record(TableRecord* record) {
		typename std::unordered_map<TableRecord*, std::pair<Key, SubKey> >::iterator it = record2key_.find(record);
		if (it == record2key_.end())
			return false;
		typename std::unordered_map<Key, ValueType*>::iterator kit = key2recordlist_.find(it->second.first);
		if (kit == key2recordlist_.end())
			return false;
		if (!kit->second)
			return false;
		kit->second->remove(it->second.second);
		return true;
	}

	bool commit_insert_request(const Key& key, const SubKey& sub_key, mysql_cmd_callback_func cb, void* param, long param_l) {
		TableRecord* record = get(key, sub_key);
		if (!record) record = get_new(key, sub_key);
		return state_mgr_.commit_insert(record, cb, param, param_l);
	}

	bool commit_insert_request(TableRecord* record, mysql_cmd_callback_func cb, void* param, long param_l) {
		return state_mgr_.commit_insert(record, cb, param, param_l);
	}

	bool commit_insert_request(const Key& key, const SubKey& sub_key) {
		return commit_insert_request(key, sub_key, nullptr, nullptr, 0);
	}

	bool commit_delete_request(const Key& key, const SubKey& sub_key) {
		TableRecord* record = get(key, sub_key);
		if (!record) {
			LogInfo("no record to request delete");
			return false;
		}
		return state_mgr_.commit_delete(record);
	}

	bool commit_delete_request(TableRecord* record) {
		return state_mgr_.commit_delete(record);
	}

	bool commit_update_request(const Key& key, const SubKey& sub_key) {
		TableRecord* record = get(key, sub_key);
		if (!record) {
			LogError("not found record");
			return false;
		}
		return state_mgr_.commit_update(record);
	}

	bool commit_update_request(TableRecord* record) {
		return state_mgr_.commit_update(record);
	}

	void update() {
		state_mgr_.start_update();
		TableRecord* record = nullptr;
		while ((record = state_mgr_.get_update_record()) != nullptr) {
			int res = state_mgr_.update_one_record(record);
			if (res == 2) {
				remove_record(record);
			}
		}
		state_mgr_.clear();
	}

protected:
	std::unordered_map<TableRecord*, std::pair<Key, SubKey> > record2key_;
	std::unordered_map<Key, ValueType*> key2recordlist_;
	mysql_record_state_manager<TableRecord> state_mgr_;
};
