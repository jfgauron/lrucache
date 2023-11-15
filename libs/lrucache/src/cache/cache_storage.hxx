#ifndef LRUCACHE_CACHE_STORAGE_
#define LRUCACHE_CACHE_STORAGE_

#include <ctime>
#include <list>
#include <map>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "lrucache/cache_config.hxx"
#include "cache_item.hxx"

namespace lrucache {

class cache_storage {
public:
    cache_storage(cache_config config)
        : config_(config)
        , used_memory_(0) {}

    virtual ~cache_storage() {}

    enum commit_result {
        DONE_OK      = 0x0,
        NOT_FOUND    = 0x1,
        KEY_TOO_BIG  = 0x3,
        DATA_TOO_BIG = 0x4,
        WRONG_EXPIRY = 0x5
    };

    /**
     * Read cache data without changing the cache state. Reading data with
     * this method won't mark the data as most recently used.
     * 
     * @param key key pointing to the data
     * @param len[out] number of bytes returned
     * @result pointer to data read
     */
    unsigned char* read(const std::string& key, size_t& len);

    /**
     * Mark data pointed by `key` as most recently used, pushing all other
     * items further down and closer to eviction.
     * 
     * @param key key pointing to the data
     * @param read_at POSIX time at which the read commit is applied. Must be
     *                later than `last_commit_time_`.
     */
    virtual bool commit_read(const std::string& key, std::time_t read_at);

    /**
     * Write data in cache that can be retrieved with the given key.
     *
     * @param key key used to retrieve the data written.
     * @param cache_item item to write into cache
     * @param written_at POSIX time of when the write is committed.
     * @return true if write succeeds. If false, `get_error_code()` will
     *         indicate why.
     */
    virtual bool commit_write(const std::string& key,
                              const cache_item& item,
                              std::time_t written_at);

    /**
     * Purge all expired items from the cache.
     * 
     * @param purge_at POSIX time of when the data is purged.
     */
    virtual void commit_purge(std::time_t purged_at);

    /**
     * See `cache_state::read_snapshot_chunk()` for more information.
     * 
     * @param chunk_size max size of data returned. Must be big enough
     *                   to contain the biggest object the cache can
     *                   store at minimum.
     * @param item_index[in,out]
     *      item index from where to start reading. At the end of the
     *      call, item_index will hold the index of the next item to
     *      read, or -1 if all items were read.
     * @param read[out] amount of bytes returned
     * @return pointer to data read 
     */
    std::unique_ptr<unsigned char[]> read_snapshot_chunk(size_t chunk_size,
                                                         int& item_index,
                                                         size_t& read);
    
    /**
     * Delete least recently used items from cache until there is at least
     * `memory_required` amount of space available.
     * 
     * @param memory_required  amount of memory in bytes to make available.
     */
    virtual void evict_lru_data(const std::string& key, int required_memory);
    
    /**
     * Reset the cache data. Use with caution.
     */
    virtual void clear();

    /**
     * Get cache_item pointed by key.
     * 
     * @param key key pointing to cache_item
     * @param when at which point in time
     * @return pointer to cache_item
    */
    virtual cache_item* get_item(const std::string& key, std::time_t when);

    /**
     * Return an approximation of the memory required to store an item.
     * 
     * @param key key pointing to the data
     * @param data_size size of the data pointed by the key
     * @return size in bytes
     */
    size_t get_item_size(const std::string& key, size_t data_size);

    /**
     * Return a code indicating why the previous commit operation failed.
     * 
     * @return DONE_OK: operation successfully completed.
     *         NOT_FOUND: when key isn't found in cache
     *         KEY_TOO_BIG: when size of key is bigger than config.max_key_size
     *         DATA_TOO_BIG: when item data is bigger than config.max_item_size
     *         WRONG_EXPIRY: expiry date is before commit time
     */
    commit_result get_commit_code();

protected:
    virtual cache_item* insert_item(const std::string& key,
                                    const cache_item& item,
                                    std::time_t written_at);

    virtual cache_item* update_item(const std::string& key,
                                    const cache_item& item,
                                    std::time_t written_at);

    virtual std::list<cache_item>::iterator remove_item(
        const std::string& key);

    void mark_as_recently_used(const std::string& key, std::time_t when);

    void update_item_expiry_bucket(const std::string& key,
                                   std::time_t old_expiry,
                                   std::time_t new_expiry);

    int get_required_memory(const std::string& key,
                            const cache_item& item,
                            std::time_t written_at);

    std::time_t next_purge_time(std::time_t current, int interval);

    struct internal_iterators {
        std::list<cache_item>::iterator
            item_iter;
        std::map<std::time_t, std::list<const std::string*>>::iterator
            expiry_map_iter;
        std::list<const std::string*>::iterator
            expiry_bucket_iter;
    };

    // cache settings
    cache_config config_;

    // source of truth for keys
    std::unordered_map<std::string, internal_iterators> item_map_;

    // source of truth for cache_item
    std::list<cache_item> items_;

    // group items by expiry date for a fast purge
    std::map<std::time_t, std::list<const std::string*>> expiry_buckets_;

    // amount of memory used by items in cache
    size_t used_memory_;

    // reason for last commit result
    commit_result commit_code_;

    // posix time of last commit
    std::time_t last_commit_time_;
};

}

#endif // LRUCACHE_CACHE_STORAGE_
