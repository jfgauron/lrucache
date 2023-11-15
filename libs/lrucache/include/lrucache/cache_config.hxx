#ifndef LRUCACHE_CACHE_CONFIG_
#define LRUCACHE_CACHE_CONFIG_

#include <stdlib.h>
#include <string>

namespace lrucache {

class cache_config {
public:
    static cache_config from_file(std::string path);

    std::string endpoint() {
        return host + ":" + std::to_string(port);
    }

    std::string raft_endpoint() {
        return raft_host + ":" + std::to_string(raft_port);
    }

    // [server]

    // address the cache will be listening on
    std::string host;
    // port the cache will be listening on
    unsigned short int port;

    // [cluster]
    
    // address and port of one node in the cluster
    std::string discovery_address;
    unsigned short int discovery_port;
    // fallback if the discovery_address is unavailable
    std::string fallback_address;
    unsigned short int fallback_port;

    // [cache]

    // amount in bytes that the cache will keep until eviction.
    size_t cache_size;
    // max size for an individual object in bytes.
    size_t max_item_size;
    // max key size.
    size_t max_key_size;
    // time in seconds before each cache purge of expired items.
    int purge_interval;

    // [raft]

    // address and port of the raft server
    std::string raft_host;
    unsigned short int raft_port;
    //time in ms before leader checks on a follower.
    int heart_beat_interval;
    // lower bound time (ms) before initiating leader election.
    int election_timeout_lower_bound;
    // upper bound time (ms) before initiating leader election.
    int election_timeout_upper_bound;
    // amount of trailing logs to keep. if a follower lags by
    // more than this amount they will have to recover using a
    // snapshot.
    int reserved_log_items;
    // number of logs required before creating a new snapshot.
    int snapshot_distance;
    // client timeout in ms
    int client_req_timeout;
};

} // namespace lrucache

#endif // LRUCACHE_CACHE_CONFIG_
