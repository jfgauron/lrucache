#ifndef LRUCACHE_CACHE_ITEM_
#define LRUCACHE_CACHE_ITEM_

#include <ctime>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

typedef std::vector<unsigned char> item_data;

namespace lrucache {

struct cache_item {
    cache_item(unsigned char* bytes, size_t len, std::time_t exp)
        : data_size(len), expires_at(exp), key(nullptr)
    {
        data = std::make_shared<item_data>(len);
        std::memcpy(data.get()->data(), bytes, len);
    }

    bool is_expired(std::time_t checked_at) const {
        return expires_at <= checked_at;
    }

    size_t size() {
        size_t result = 0;
        result += sizeof(key->size());
        if (key)
            result += key->size();
        result += sizeof(data_size);
        result += data_size;
        result += sizeof(expires_at);
        return result;
    }

    const std::string* key;
    std::shared_ptr<item_data> data;
    size_t data_size;
    std::time_t expires_at;
};

}

#endif // LRUCACHE_CACHE_ITEM_
