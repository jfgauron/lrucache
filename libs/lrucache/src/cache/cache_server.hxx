#ifndef LRUCACHE_CACHE_SERVER_H_
#define LRUCACHE_CACHE_SERVER_H_

#include "ctpl/ctpl_stl.h"
#include "sockpp/tcp_acceptor.h"

#include "lrucache/cache_config.hxx"

namespace lrucache {

class cache_server {
public:
    cache_server(cache_config config, int server_id);

    void run();

    static void handle_connection(int id, sockpp::tcp_socket& socket);
private:
    int server_id_;
    cache_config config_;
    ctpl::thread_pool thread_pool_;
};

} // namespace lrucache

#endif // LRUCACHE_CACHE_SERVER_H_