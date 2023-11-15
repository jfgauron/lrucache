#ifndef LRUCACHE_CLIENT_H_
#define LRUCACHE_CLIENT_H_

#include <memory>
#include <set>

#include "lrucache/client_config.hxx"

namespace lrucache {

class endpoint;

class client {
public:
    client(client_config config);

    bool discover();

    bool write(unsigned char data[], size_t len);

    std::unique_ptr<unsigned char[]> read(const std::string& key, size_t& len);
private:
    
    // client settings such as where to look up for the cluster
    client_config config_;

    // set of cache endpoints discovered by the client sorted by distance
    std::set<endpoint> endpoints_;
};

} // namespace lrucache

#endif // LRUCACHE_CLIENT_H_