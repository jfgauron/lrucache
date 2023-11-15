#include "utilities.hxx"

lrucache::cache_config build_default_cache_config()
{
    lrucache::cache_config config;
    config.cache_size = 150;
    config.max_item_size = 90;
    config.max_key_size = 20;
    config.purge_interval = 30;
    return config;
}

lrucache::cache_item create_item(size_t size, std::time_t expires_at)
{
    unsigned char data[size];
    return lrucache::cache_item(data, size, expires_at);
}

std::vector<lrucache::cache_item> read_snapshot_data(unsigned char data[],
                                                     size_t size)
{
    std::vector<lrucache::cache_item> result;
    size_t key_size;
    size_t data_size;
    std::time_t expires_at;
    unsigned char* data_ptr;
    size_t read = 0;

    while (read < size) {
        memcpy(&key_size, data, sizeof(key_size));
        data += sizeof(key_size);
        data += key_size; // ignore key
        memcpy(&data_size, data, sizeof(data_size));
        data += sizeof(data_size);
        data_ptr = data;
        data += data_size;
        memcpy(&expires_at, data, sizeof(expires_at));
        data += sizeof(expires_at);

        lrucache::cache_item item(data_ptr, data_size, expires_at);
        read += item.size() + key_size;
        result.push_back(std::move(item));
    }

    return result;
}