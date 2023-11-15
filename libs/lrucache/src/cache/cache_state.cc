#include "cache_state.hxx"

#include <iostream>
#include <cstring>
#include <mutex>

namespace lrucache {

std::unique_ptr<unsigned char[]> cache_state::read(const std::string& key,
                                                   size_t& len)
{
    std::unique_ptr<unsigned char[]> result = nullptr;
    read_then(key, [&result, &len](unsigned char* data, size_t n) {
        if (!data)
            return;
        result = std::make_unique<unsigned char[]>(n);
        std::memcpy(result.get(), data, n);
        len = n;
    });
    return result;
}

void cache_state::read_then(const std::string& key,
                            std::function<void(unsigned char*, size_t)> then)
{
    std::lock_guard<std::mutex> lock(cache_lock_);

    size_t len = 0;
    unsigned char* data = nullptr;
    cache_storage* storage = &storage_;

    if (snapshot_in_progress_) {
        storage = &snapshot_storage_;
    }

    data = storage->read(key, len);
    then(data, len);
}

bool cache_state::commit_read(const std::string& key, std::time_t read_at)
{
    std::lock_guard<std::mutex> lock(cache_lock_);

    bool result = storage()->commit_read(key, read_at);
    commit_code_ = storage()->get_commit_code();
    return result;
}

bool cache_state::commit_write(const std::string& key,
                               const cache_item& item,
                               std::time_t written_at)
{
    std::lock_guard<std::mutex> lock(cache_lock_);

    bool result = storage()->commit_write(key, std::move(item), written_at);
    commit_code_ = storage()->get_commit_code();
    return result;
}

void cache_state::commit_purge_expired(std::time_t purge_at)
{
    storage()->commit_purge(purge_at);
}

void cache_state::begin_snapshot()
{
    std::lock_guard<std::mutex> lock(cache_lock_);
    snapshot_in_progress_ = true;
}

std::unique_ptr<unsigned char[]> cache_state::read_snapshot_chunk(
        size_t chunk_size, int& item_index, size_t& read)
{
    return storage_.read_snapshot_chunk(chunk_size, item_index, read);
}

void cache_state::end_snapshot()
{
    std::lock_guard<std::mutex> lock(cache_lock_);

    size_t len = 0;
    const auto& events = snapshot_storage_.events();
    for (const auto& event : events) {
        switch (event.type) {
            case snapshot_event::type::READ:
                storage_.commit_read(*event.key, event.created_at);
                break;
            case snapshot_event::type::WRITE:
                storage_.commit_write(*event.key, *event.item, event.created_at);
                break;
            case snapshot_event::type::PURGE:
                storage_.commit_purge(event.created_at);
                break;
        };
    }

    snapshot_storage_.clear();
    snapshot_in_progress_ = false;
}

cache_storage::commit_result cache_state::get_commit_code()
{
    return commit_code_;
}

cache_storage *cache_state::storage()
{
    if (snapshot_in_progress_)
        return &snapshot_storage_;
    return &storage_;
}

} // namespace lrucache