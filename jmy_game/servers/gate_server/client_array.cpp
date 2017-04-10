#include "client_array.h"
#include "../libjmy/jmy_mem.h"

ClientArray::ClientArray() : info_array_(nullptr), start_id_(0), info_size_(0), inited_(false)
{
}

ClientArray::~ClientArray()
{
	clear();
}

bool ClientArray::init(int array_size, int start_id)
{
	if (array_size <= 0 || start_id <= 0) {
		return false;
	}
	if (info_array_) return true;
	if (inited_) return true;
	info_array_ = (ClientInfo**)jmy_mem_malloc(sizeof(ClientInfo*)*array_size);
	int i = 0;
	for (; i<array_size; ++i) {
		info_array_[i] = jmy_mem_malloc<ClientInfo>();
		info_array_[i]->id = start_id + i;
		free_ids_.push_back(start_id+i);
	}
	info_size_ = array_size;
	start_id_ = start_id;
	inited_ = true;
	return true;
}

void ClientArray::clear()
{
	if (info_array_) {
		for (int i=0; i<info_size_; ++i) {
			if (info_array_[i]) {
				jmy_mem_free<ClientInfo>(info_array_[i]);
			}
		}
		jmy_mem_free(info_array_);
	}
	free_ids_.clear();
	used_ids_.clear();
	start_id_ = 0;
	info_size_ = 0;
	inited_ = false;
}

void ClientArray::reset()
{
}

ClientInfo* ClientArray::getFree()
{
	if (!inited_) return nullptr;
	if (free_ids_.size() == 0)
		return nullptr;
	
	int id = free_ids_.front();
	free_ids_.pop_front();
	ClientInfo* info = info_array_[id-start_id_];
	info->used = true;
	used_ids_.insert(id);
	return info;
}

ClientInfo* ClientArray::get(int id)
{
	if (!inited_) return nullptr;
	if (id<start_id_ || id>start_id_+info_size_)
		return nullptr;
	if (used_ids_.find(id) != used_ids_.end())
		return nullptr;

	return info_array_[id-start_id_];
}

bool ClientArray::free(ClientInfo* info)
{
	if (!inited_) return false;
	if (!info->used)
		return false;

	free_ids_.push_front(info->id);
	used_ids_.erase(info->id);
	info->used = false;
	return true;
}
