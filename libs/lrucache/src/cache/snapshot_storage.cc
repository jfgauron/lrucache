#include "snapshot_storage.hxx"

namespace lrucache {

bool snapshot_storage::commit_read(const std::string &key, std::time_t read_at)
{
    auto item = get_item(key, read_at);

    if (!item) {
        commit_code_ = commit_result::NOT_FOUND;
        return false;
    }

    auto type = snapshot_event::type::READ;
    snapshot_event event(type, item->key, item, 0, read_at);
    events_.push_back(std::move(event));
    commit_code_ = commit_result::DONE_OK;
    return true;
}

void snapshot_storage::commit_purge(std::time_t purged_at)
{
    auto type = snapshot_event::type::PURGE;
    snapshot_event event(type, nullptr, nullptr, 0, purged_at);
    events_.push_back(std::move(event));
    commit_code_ = commit_result::DONE_OK;
}

void snapshot_storage::clear()
{
    cache_storage::clear();
    events_.clear();
}

cache_item* snapshot_storage::get_item(const std::string& key, std::time_t at)
{
    auto item = cache_storage::get_item(key, at);
    if (!item) {
        return storage_->get_item(key, at);
    }
    return item;
}

void snapshot_storage::evict_lru_data(int memory_required)
{
    return;
}

const std::vector<snapshot_event>& snapshot_storage::events()
{
    return events_;
}

cache_item* snapshot_storage::insert_item(const std::string &key,
                                                const cache_item &inserted_item,
                                                std::time_t written_at)
{
    auto item = cache_storage::insert_item(key, inserted_item, written_at);

    auto type = snapshot_event::type::WRITE;
    snapshot_event event(type, item->key, item, item->expires_at, written_at);
    events_.push_back(std::move(event));
    return item;
}

cache_item* snapshot_storage::update_item(const std::string &key,
                                          const cache_item &updated_item,
                                          std::time_t written_at)
{
    cache_item* item = nullptr;
    if (cache_storage::get_item(key, written_at)) {
        item = cache_storage::update_item(key, updated_item, written_at);
    } else {
        item = cache_storage::insert_item(key, updated_item, written_at);
    }

    auto type = snapshot_event::type::WRITE;
    snapshot_event event(type, item->key, item, item->expires_at, written_at);
    events_.push_back(std::move(event));
    return item;
}

}
