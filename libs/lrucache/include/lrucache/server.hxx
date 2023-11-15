#ifndef LRUCACHE_SERVER_H_
#define LRUCACHE_SERVER_H_

#include <memory>

#include "cache_config.hxx"

namespace lrucache {


class raft_server;
class cache_server;

class server {
public:
    server(cache_config config);
    ~server();

    void init();

    void run();

    void backgroud_run();

private:
    cache_config config_;

    std::unique_ptr<raft_server> raft_;
    std::unique_ptr<cache_server> cache_;
};

}

#endif // LRUCACHE_SERVER_H_