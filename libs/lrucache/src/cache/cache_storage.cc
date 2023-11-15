#include "cache_storage.hxx"

#include <iostream>

namespace lrucache {

unsigned char* cache_storage::read(const std::string& key, size_t& len)
{
    auto item = get_item(key, std::time(nullptr));
    if (!item) {
        return nullptr;
    }
    len = item->data_size;
    return item->data.get()->data();
}

bool cache_storage::commit_read(const std::string& key, std::time_t read_at)
{
    auto item = get_item(key, read_at);
    if (!item) {
        commit_code_ = commit_result::NOT_FOUND;
        return false;
    }
    commit_code_ = commit_result::DONE_OK;
    mark_as_recently_used(key, read_at);
    return true;
}

bool cache_storage::commit_write(const std::string& key,
                                 const cache_item& item,
                                 std::time_t written_at)
{
    if (key.size() > config_.max_key_size) {
        commit_code_ = commit_result::KEY_TOO_BIG;
        return false;
    }

    if (item.data_size < 0) {
        commit_code_ = commit_result::DATA_TOO_BIG;
        return false;
    }

    if (item.is_expired(written_at)) {
        commit_code_ = commit_result::WRONG_EXPIRY;
        return false;
    }

    int required_memory = get_required_memory(key, item, written_at);
    evict_lru_data(key, required_memory);

    auto old_item = get_item(key, written_at);
    if (old_item) {
        update_item(key, item, written_at);
    } else {
        insert_item(key, item, written_at);
    }
    commit_code_ = commit_result::DONE_OK;
    return true;
}

void cache_storage::commit_purge(std::time_t purged_at)
{
    std::time_t next = next_purge_time(purged_at, config_.purge_interval);
    std::time_t prev = next - config_.purge_interval;

    auto i = expiry_buckets_.begin();
    while (i != expiry_buckets_.end()) {
        auto current = i++;
        if (current->first > prev) {
            return;
        }
        auto j = current->second.begin();
        while (j != current->second.end()) {
            auto iter = j++;
            remove_item(**iter);
        }
    }
}

void cache_storage::mark_as_recently_used(const std::string& key,
                                          std::time_t when)
{
    auto iter = item_map_.find(key);
    if (iter != item_map_.end()) {
        items_.splice(items_.begin(), items_, iter->second.item_iter);
    }
}

std::unique_ptr<unsigned char[]> cache_storage::read_snapshot_chunk(
        size_t chunk_size, int& item_index, size_t& read)
{
    auto result = std::make_unique<unsigned char[]>(chunk_size);
    auto ptr = result.get();
    read = 0;

    auto it = items_.begin();
    std::advance(it, std::min(items_.size(), (size_t)item_index));
    while (it != items_.end()) {
        size_t size = it->size();
        if (read + size > chunk_size) {
            return result;
        }

        // key
        auto key_size = it->key->size();
        memcpy(ptr, &key_size, sizeof(key_size));
        ptr += sizeof(key_size);
        memcpy(ptr, it->key->data(), key_size);
        ptr += it->key->size();

        // data
        memcpy(ptr, &it->data_size, sizeof(it->data_size));
        ptr += sizeof(it->data_size);
        memcpy(ptr, it->data.get()->data(), it->data_size);
        ptr += it->data_size;

        // expiry
        memcpy(ptr, &it->expires_at, sizeof(it->expires_at));
        ptr += sizeof(it->expires_at);

        read += size;
        item_index++;
        it++;
    }

    item_index = -1; // done!
    return result;
}

void cache_storage::evict_lru_data(const std::string& key, int memory_required)
{
    int available_memory = config_.cache_size - used_memory_;
    if (available_memory >= memory_required) {
        return;
    }

    size_t removed_memory = 0;
    auto it = items_.rbegin();
    while (it != items_.rend()) {
        if (*it->key == key) {
            it++;
            continue;
        }
        size_t item_size = get_item_size(*it->key, it->data_size);
        it = std::make_reverse_iterator(remove_item(*it->key));
        available_memory += item_size;
        if (available_memory >= memory_required) {
            return;
        }
    }
}

void cache_storage::clear()
{
    items_.clear();
    item_map_.clear();
    expiry_buckets_.clear();
    used_memory_ = 0;
}

cache_item* cache_storage::get_item(const std::string& key, std::time_t when)
{    
    auto iter = item_map_.find(key);
    if (iter != item_map_.end()) {
        if (!iter->second.item_iter->is_expired(when)) {
            return &(*iter->second.item_iter);
        }
    }
    return nullptr;
}

size_t cache_storage::get_item_size(const std::string& key, size_t data_size)
{
    size_t result = 0;
    result += sizeof(cache_item);
    result += sizeof(std::list<cache_item>::iterator);
    result += data_size;
    result += key.size();
    return result;
}

cache_storage::commit_result cache_storage::get_commit_code()
{
    return commit_code_;
}

cache_item* cache_storage::insert_item(const std::string& key,
                                       const cache_item& item,
                                       std::time_t written_at)
{
    // insert into items_
    auto item_iter = items_.insert(items_.begin(), item);

    // make sure expiry bucket already exist
    auto purge_at = next_purge_time(item.expires_at, config_.purge_interval);
    auto expiry_map_iter = expiry_buckets_.emplace_hint(
        expiry_buckets_.end(),
        purge_at,
        std::move(std::list<const std::string*>()));
    
    // insert temporary key into expiry_buckets_
    auto* expiry_bucket = &expiry_map_iter->second;
    auto expiry_item_iter = expiry_bucket->insert(
        expiry_bucket->end(), nullptr);

    // insert into item_map_
    internal_iterators iters;
    iters.item_iter = item_iter;
    iters.expiry_map_iter = expiry_map_iter;
    iters.expiry_bucket_iter = expiry_item_iter;
    auto item_map_iter = item_map_.insert({key, iters});

    // update keys
    (*expiry_item_iter) = &item_map_iter.first->first;
    item_iter->key = &item_map_iter.first->first;

    // update used_memory_
    size_t item_size = get_item_size(key, item_iter->data_size);
    used_memory_ += item_size;

    return &(*item_iter);
}

cache_item* cache_storage::update_item(const std::string& key,
                                       const cache_item& item,
                                       std::time_t written_at)
{
    auto item_iter = item_map_.find(key)->second.item_iter;
    const std::string* old_key = item_iter->key;
    std::time_t old_expiry = item_iter->expires_at;

    // update memory used
    auto new_item_size = get_item_size(key, item.data_size);
    auto old_item_size = get_item_size(key, item_iter->data_size);
    used_memory_ += new_item_size - old_item_size;

    // update item
    (*item_iter) = item;
    (*item_iter).key = old_key;
    items_.splice(items_.begin(), items_, item_iter);
    update_item_expiry_bucket(key, old_expiry, item.expires_at);

    return &(*item_iter);
}

std::list<cache_item>::iterator cache_storage::remove_item(
        const std::string &key)
{
    auto item_map_iter = item_map_.find(key);
    if (item_map_iter != item_map_.end()) {
        auto iters = item_map_iter->second;
        auto item_iter = iters.item_iter;
        auto expiry_map_iter = iters.expiry_map_iter;
        auto expiry_bucket_iter = iters.expiry_bucket_iter;
        size_t size = get_item_size(*item_iter->key, item_iter->data_size);

        item_iter = items_.erase(item_iter);
        item_map_.erase(item_map_iter);
        expiry_map_iter->second.erase(expiry_bucket_iter);
        if (expiry_map_iter->second.size() == 0) {
            expiry_buckets_.erase(expiry_map_iter);
        }

        used_memory_ -= size;
        return item_iter;
    }
    return items_.end();
}

void cache_storage::update_item_expiry_bucket(const std::string& key,
                                              std::time_t old_expiry,
                                              std::time_t new_expiry)
{
    std::time_t expiry1 = next_purge_time(old_expiry, config_.purge_interval);
    std::time_t expiry2 = next_purge_time(new_expiry, config_.purge_interval);

    if (expiry1 == expiry2) {
        return;
    }

    auto item_map_iter = item_map_.find(key);
    auto iters = item_map_iter->second;
    auto old_expiry_map_iter = iters.expiry_map_iter;
    auto old_expiry_bucket_iter = iters.expiry_bucket_iter;
    auto* old_bucket = &old_expiry_map_iter->second;

    // make sure destination expirty bucket already exist
    auto new_expiry_map_iter = expiry_buckets_.emplace_hint(
        expiry_buckets_.end(),
        expiry2,
        std::list<const std::string*>());
    
    // insert into new expiry bucket and update iterators
    auto* new_bucket = &new_expiry_map_iter->second;
    auto new_expiry_bucket_iter = new_bucket->insert(
        new_bucket->end(),
        iters.item_iter->key);
    iters.expiry_bucket_iter = new_expiry_bucket_iter;
    iters.expiry_map_iter = new_expiry_map_iter;
    item_map_iter->second = iters;

    // delete previous iterators
    old_expiry_map_iter->second.erase(old_expiry_bucket_iter);
    if (old_expiry_map_iter->second.size() == 0) {
        expiry_buckets_.erase(old_expiry_map_iter);
    }
}

int cache_storage::get_required_memory(const std::string& key,
                                       const cache_item& item,
                                       std::time_t written_at)
{
    size_t item_size = get_item_size(key, item.data_size);
    size_t old_size = 0;

    auto old_item = get_item(key, written_at);
    if (old_item) {
        old_size = get_item_size(key, old_item->data_size);
    }
    return item_size - old_size;
}

std::time_t cache_storage::next_purge_time(std::time_t current, int interval)
{
    int remainder = current % interval;
    int difference = interval - remainder;
    return current + difference;
}

}
