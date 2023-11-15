#ifndef LRUCACHE_SNAPSHOT_EVENT_
#define LRUCACHE_SNAPSHOT_EVENT_

#include <string>
#include <ctime>

#include "cache_item.hxx"

namespace lrucache {

struct snapshot_event {
    enum type {
        READ  = 0x1,
        WRITE = 0x2,
        PURGE = 0x3
    };

    snapshot_event() {}

    snapshot_event(type type, const std::string* key, cache_item* item,
                   std::time_t expires_at, std::time_t created_at)
        : type(type), key(key), item(item), expires_at(expires_at)
        , created_at(created_at) {}

    type type;
    const std::string* key;
    const cache_item* item;
    std::time_t expires_at;
    std::time_t created_at;
};

}

#endif // LRUCACHE_SNAPSHOT_EVENT_
