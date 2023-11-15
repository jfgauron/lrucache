#ifndef LRUCACHE_ENDPOINT_H_
#define LRUCACHE_ENDPOINT_H_

#include <string>

namespace lrucache {

struct endpoint {
public:
    bool operator<(const endpoint &node) const;

    std::string host;
    unsigned short int port;
    double distance_;
};

} // namespace lrucache

#endif // LRUCACHE_ENDPOINT_H_