#ifndef LRUCACHE_CACHE_CONFIG_
#define LRUCACHE_CACHE_CONFIG_

#include <stdlib.h>
#include <string>

namespace lrucache {

class client_config {
public:
    static client_config from_file(std::string path);

    // [cluster]
    
    // address and port of one node in the cluster
    std::string discovery_address;
    unsigned short int discovery_port;

    // fallback if discovery_address is unavailable
    std::string fallback_address;
    unsigned short int fallback_port;
};

} // namespace lrucache

#endif // LRUCACHE_CACHE_CONFIG_
