#ifndef LRUCACHE_UTILITIES_H_
#define LRUCACHE_UTILITIES_H_

#include <vector>

#include "lrucache/cache_config.hxx"
#include "cache/cache_item.hxx"

lrucache::cache_config build_default_cache_config();

lrucache::cache_item create_item(size_t size, std::time_t expires_at);

std::vector<lrucache::cache_item> read_snapshot_data(unsigned char data[],
                                                     size_t size);

#endif // LRUCACHE_UTILITIES_H_