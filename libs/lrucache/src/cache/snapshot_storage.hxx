#ifndef LRUCACHE_SNAPSHOT_STORAGE_
#define LRUCACHE_SNAPSHOT_STORAGE_

#include <vector>

#include "cache_storage.hxx"
#include "snapshot_event.hxx"

namespace lrucache {

class snapshot_storage : public cache_storage {
public:
    snapshot_storage(cache_config config, cache_storage* storage)
        : cache_storage(config)
        , storage_(storage) {}

    virtual ~snapshot_storage() {}

    /**
     * Does not change the cache state aside from adding a snapshot_event
     * to `events_`.
     * 
     * @param key key pointing to the data
     * @param read_at POSIX time at which the read commit is applied. Must be
     *                later than `last_commit_time_`
    */
    virtual bool commit_read(const std::string& key, std::time_t read_at);

    /**
     * Does not change the cache state aside from adding a snapshot_event
     * to `events_`.
     * 
     * @param purge_at POSIX time of when the data is purged.
     */
    virtual void commit_purge(std::time_t purged_at);

    /**
     * Same as cache_storage::clear() but also destroy `events_`.
    */
    virtual void clear();

    /**
     * Same as cache_storage::get_item() but first check to see if the data
     * is present in its own snapshot storage before looking into the cache
     * storage.
     * 
     * @param key key pointing to cache_item
     * @param when at which point in time
     * @return pointer to cache_item
     */
    virtual cache_item* get_item(const std::string& key, std::time_t when);

    /**
     * Does nothing, eviction is disabled when snapshot is in progress
     * 
     * @param memory_required not used.
     */
    virtual void evict_lru_data(int memory_required);

    const std::vector<snapshot_event>& events();

protected:
    virtual cache_item* insert_item(const std::string& key,
                                    const cache_item& inserted_item,
                                    std::time_t written_at);

    virtual cache_item* update_item(const std::string& key,
                                    const cache_item& updated_item,
                                    std::time_t written_at);

    // keeps track of commits to be applied once snapshot is completed
    std::vector<snapshot_event> events_;

    // frozen snapshot data 
    cache_storage* storage_;
};

}

#endif // LRUCACHE_SNAPSHOT_STORAGE_
