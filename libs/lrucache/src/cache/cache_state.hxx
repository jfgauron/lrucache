#ifndef LRUCACHE_CACHE_STATE_H_
#define LRUCACHE_CACHE_STATE_H_

#include <atomic>
#include <functional>
#include <mutex>

#include "lrucache/cache_config.hxx"
#include "cache_storage.hxx"
#include "snapshot_storage.hxx"

namespace lrucache {

class cache_state {
public:
    cache_state(cache_config config)
        : snapshot_in_progress_(false)
        , storage_(config)
        , snapshot_storage_(config, &storage_)
        , config_(config) {}

    /**
     * Read data in cache at the given key.
     * 
     * @param key key used to retrieve the data.
     * @param len[out] size of data read.
     * @return pointer to the data read, or nullptr if no data found
     */
    std::unique_ptr<unsigned char[]> read(const std::string& key, size_t& len);

    /**
     * Read data in cache at given key and execute the callback function
     * with returned data as is without an extra copy.
     * 
     * This is useful to avoid the cost incurred by `read()` copy of data.
     * With this method, no copying is necessary and the data read is
     * thread-safe until the end of the callback.
     * 
     * @param key key used to retrieve the data.
     * @param then callback function taking the read data and the
     *             length of the data. If no data is found, the callback
     *             will receive a nullptr instead.
     */
    void read_then(const std::string& key,
                   std::function<void(unsigned char*, size_t)> then);

    /**
     * Update the cache state so that the data pointed by the key is put at
     * the back of the eviction queue.
     * 
     * @param key key for the data to read.
     * @param read_at POSIX time of when the read is committed.
     * @return true if read succeeds. If false, `get_error_code()` will
     *         indicate why.
     */
    bool commit_read(const std::string& key, std::time_t read_at);

    /**
     * Write data in cache that can be retrieved with the given key.
     *
     * @param key key used to retrieve the data written.
     * @param cache_item item to write into cache
     * @param written_at POSIX time of when the write is committed.
     * @return true if write succeeds. If false, `get_error_code()` will
     *         indicate why.
     */
    bool commit_write(const std::string& key,
                      const cache_item& item,
                      std::time_t written_at);

    /**
     * Purge all expired items from the cache.
     * 
     * @param purge_at POSIX time of when the data is purged.
     */
    void commit_purge_expired(std::time_t purge_at);

    /**
     * Instruct the cache state that a snapshot of its data is in progress.
     * 
     * When a snapshot is in progress, the cache data is frozen in time
     * until `end_snapshot()` is called. Reads and writes are still possible
     * but will go to a temporary data storage instead. Evictions are
     * disabled during the snapshot process for performance reasons. Once
     * the snapshot is terminated, evictions will be applied in the right
     * order as expected if no snapshot happened.
     * 
     * For consumers, this entire process should be transparent except that
     * reads could return stale data until evictions are enabled again.
     * 
     */
    void begin_snapshot();

    /**
     * Read up to `chunk_size` of data from the frozen cache data during
     * snapshot process. Each cache item will be written in this format:
     * 
     *  <key size>   <key data>    <data size>     <data>       <expiry>
     *   8 bytes   key_size bytes    8 bytes    data_size bytes  8 bytes
     * 
     * The data will also be listed in order from most recently used to
     * least recently used.
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
     * Terminate the snapshot process.
     * 
     * See `begin_snapshot()` for more information.
     */
    void end_snapshot();

    /**
     * Return a code indicating why the previous commit operation failed.
     * 
     * @return DONE_OK: operation successfully completed.
     *         NOT_FOUND: when key isn't found in cache
     *         KEY_TOO_BIG: when size of key is bigger than config.max_key_size
     *         DATA_TOO_BIG: when item data is bigger than config.max_item_size
     *         WRONG_EXPIRY: expiry date is before commit time
     */
    cache_storage::commit_result get_commit_code();


private:
    /**
     * Get current cache_storage strategy.
     * 
     * @return either `cache_storage` or `snapshot_storage` based
     *         `snapshot_in_progress_` value.
     */
    cache_storage* storage();

    // true if snapshot is in progress
    std::atomic<bool> snapshot_in_progress_;

    // storage used during normal operations
    cache_storage storage_;

    // storage used while snapshot is in progress
    snapshot_storage snapshot_storage_;

    // cache settings
    cache_config config_;

    // reason for last commit result
    cache_storage::commit_result commit_code_;

    // Mutex for cache operations.
    std::mutex cache_lock_;
};

} // namespace lrucache

#endif // LRUCACHE_CACHE_STATE_H_