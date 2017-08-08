#pragma once

#include <list>
#include <set>
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
			//mysql_cmd_callback_func cb;
		  	//void* param;
			//long l_param;
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
	mysql_record_state_manager() {}
	~mysql_record_state_manager() {}

	void clear() {
		states_.clear();
	}

	size_t size() {
		return states_.size();
	}

	record_state_info* get_state(TableRecord* t) {
		typename std::unordered_map<TableRecord*, record_state_info>::iterator it = states_.find(t);
		if (it == states_.end()) {
			return nullptr;
		}
		return &it->second;
	}

	bool commit_insert(TableRecord* record/*, mysql_cmd_callback_func cb, void* param, long param_l*/) {
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
			//state.data.insert_data.cb = cb;
			//state.data.insert_data.param = param;
			//state.data.insert_data.l_param = param_l;
			states_.insert(std::make_pair(record, state));
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
		return record;
	}

	int update_one_record(TableRecord* record) {
		if (curr_iter_->second.state == MYSQL_TABLE_RECORD_STATE_INSERT) {
			if (!record->insert_request(
						/*curr_iter_->second.data.insert_data.cb,
						curr_iter_->second.data.insert_data.param,
						curr_iter_->second.data.insert_data.l_param*/)) {
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

	void clear() {
		typename BiMap<Key, TableRecord*>::one_map_type::iterator it = key_record_map_.one_map_begin();
		for (; it!=key_record_map_.one_map_end(); ++it) {
			if (it->second) {
				jmy_mem_free(it->second);
			}
		}
		key_record_map_.clear();
	}

	TableRecord* get_new_no_insert() {
		return jmy_mem_malloc<TableRecord>();
	}

	bool insert_new(const Key& key, TableRecord* record) {
		TableRecord* tmp_record = nullptr;
		if (key_record_map_.find_1(key, tmp_record))
			return false;
		key_record_map_.insert(key, record);
		return true;
	}
	
	TableRecord* get(const Key& key) {
		TableRecord* record = nullptr;
		if (!key_record_map_.find_1(key, record))
			return nullptr;
		return record;
	} 

	TableRecord* get_new(const Key& key) {
		TableRecord* record = jmy_mem_malloc<TableRecord>();
		key_record_map_.insert(key, record);
		return record;
	}

	bool remove(const Key& key) {
		TableRecord* record = nullptr;
		if (!key_record_map_.find_1(key, record)) {
			return false;
		}
		if (record) {
			jmy_mem_free(record);
		}
		key_record_map_.remove_1(key);
		return true;
	}

	bool remove(TableRecord* record) {
		if (!key_record_map_.find_2(record, tmp_key_)) {
			return false;
		}
		jmy_mem_free(record);
		key_record_map_.remove_2(record);
		return true;
	}

	bool commit_insert_request(const Key& key/*, mysql_cmd_callback_func cb, void* param, long param_l*/) {
		TableRecord* record = get(key);
		if (!record) record = get_new(key);
		return state_mgr_.commit_insert(record/*, cb, param, param_l*/);
	}

	bool commit_insert_request(TableRecord* record/*, mysql_cmd_callback_func cb, void* param, long param_l*/) {
		return state_mgr_.commit_insert(record/*, cb, param, param_l*/);
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
			if (!state_mgr_.next_update_record()) {
				break;
			}
		}
		state_mgr_.clear();
	}

protected:
	BiMap<Key, TableRecord*> key_record_map_;
	mysql_record_state_manager<TableRecord> state_mgr_;
	Key tmp_key_;
};

template <typename TableRecord, typename Key, typename Key2>
class mysql_records_manager2
{
public:
	mysql_records_manager2() {}
	~mysql_records_manager2() {}

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

	TableRecord* get_new_by_key_and_key2(const Key& key, const Key2& key2) {
		TableRecord* record = jmy_mem_malloc<TableRecord>();
		key_record_map_.insert(key, record);
		key2_record_map_.insert(key2, record);
		return record;
	}

	bool insert_key_record(const Key& key, TableRecord* record) {
		// must exists
		if (!key2_record_map_.find_2(record, tmp_key2_))
			return false;
		TableRecord* tmp_record = nullptr;
		if (!key_record_map_.find_1(key, tmp_record)) {
			key_record_map_.insert(key, record);
		}
		return true;
	}

	bool insert_key2_record(const Key2& key2, TableRecord* record) {
		// must exists
		if (!key_record_map_.find_2(record, tmp_key_))
			return false;
		TableRecord* tmp_record = nullptr;
		if (!key2_record_map_.find_1(key2, tmp_record)) {
			key2_record_map_.insert(key2, record);
		}
		return true;
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

	bool commit_insert_request_by_key(const Key& key/*, mysql_cmd_callback_func cb, void* param, long param_l*/) {
		TableRecord* record = get_by_key(key);
		if (!record) record = get_new_by_key2(key);
		return state_mgr_.commit_insert(record/*, cb, param, param_l*/);
	}

	bool commit_insert_request_by_key2(const Key2& key2/*, mysql_cmd_callback_func cb, void* param, long param_l*/) {
		TableRecord* record = get_by_key2(key2);
		if (!record) record = get_new_by_key2(key2);
		return state_mgr_.commit_insert(record/*, cb, param, param_l*/);
	}

	bool commit_insert_request(TableRecord* record/*, mysql_cmd_callback_func cb, void* param, long param_l*/) {
		return state_mgr_.commit_insert(record/*, cb, param, param_l*/);
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
		if (state_mgr_.size() == 0)
			return;
		state_mgr_.start_update();
		TableRecord* record = nullptr;
		while ((record = state_mgr_.get_update_record()) != nullptr) {
			int res = state_mgr_.update_one_record(record);
			if (res < 0) {
				return;
			} else if (res == 2) {
				remove_record(record);
			}
			if (!state_mgr_.next_update_record())
				break;
		}
		state_mgr_.clear();
	}

protected:
	BiMap<Key, TableRecord*> key_record_map_;
	BiMap<Key2, TableRecord*> key2_record_map_;
	mysql_record_state_manager<TableRecord> state_mgr_;
	Key2 tmp_key2_;
	Key tmp_key_;
};

template <typename TableRecord, typename Key, typename SubKey>
class mysql_records_manager_map
{
public:
	mysql_records_manager_map() {}
	~mysql_records_manager_map() {}

	typedef mysql_records_manager<TableRecord, SubKey> ValueType;

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

	void commit_changed(const Key& key) {
		if (key_set_committed_.find(key) != key_set_committed_.end())
			return;
		key_set_committed_.insert(key);
	}

	void update() {
		if (key_set_committed_.size() == 0)
			return;
		typename std::set<Key>::iterator it = key_set_committed_.begin();
		for (; it!=key_set_committed_.end(); ++it) {
			const Key& k = *it;
			typename std::unordered_map<Key, ValueType*>::iterator kit = key2recordlist_.find(k);
			if (kit == key2recordlist_.end()) {
				LogWarn("not found changed key in recordlist");
				continue;
			}
			if (!kit->second) continue;
			kit->second->update();
		}
		key_set_committed_.clear();
	}

protected:
	std::unordered_map<Key, ValueType*> key2recordlist_;
	std::set<Key> key_set_committed_;
};
