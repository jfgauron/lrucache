#include "lrucache/cache_config.hxx"
#include "ini/ini.h"

namespace lrucache {

cache_config cache_config::from_file(std::string path)
{
    inih::INIReader r{path};
    cache_config config;

    config.host = r.Get<std::string>("server", "host");
    config.port = r.Get<unsigned short int>("server", "port");

    config.discovery_address =
            r.Get<std::string>("cluster", "discovery_address");
    config.discovery_port =
            r.Get<unsigned short int>("cluster", "discovery_port");
    config.fallback_address =
            r.Get<std::string>("cluster", "fallback_address");
    config.fallback_port =
            r.Get<unsigned short int>("cluster", "fallback_port");

    config.cache_size = r.Get<size_t>("cache", "cache_size");
    config.max_item_size = r.Get<size_t>("cache", "max_item_size");
    config.max_key_size = r.Get<size_t>("cache", "max_key_size");
    config.purge_interval = r.Get<unsigned short int>("cache", "purge_interval");

    config.raft_host = r.Get<std::string>("raft", "raft_host");
    config.raft_port = r.Get<unsigned short int>("raft", "raft_port");
    config.heart_beat_interval =
            r.Get<size_t>("raft", "heart_beat_interval");
    config.election_timeout_lower_bound =
            r.Get<size_t>("raft", "election_timeout_lower_bound");
    config.election_timeout_upper_bound =
            r.Get<size_t>("raft", "election_timeout_upper_bound");
    config.reserved_log_items =
            r.Get<size_t>("raft", "reserved_log_items");
    config.snapshot_distance =
            r.Get<size_t>("raft", "snapshot_distance");
    config.client_req_timeout =
            r.Get<size_t>("raft", "client_req_timeout");

    return config;
}

} // namespace lrucache
