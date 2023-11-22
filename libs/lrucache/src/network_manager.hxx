#ifndef LRUCACHE_NETWORK_MANAGER_H
#define LRUCACHE_NETWORK_MANAGER_H

#include "ctpl/ctpl_stl.h"
#include "sockpp/tcp_acceptor.h"

#include "request_dispatcher.hxx"

namespace lrucache {

class network_manager {
public:
    network_manager(int server_id,
                    std::string host,
                    unsigned short port,
                    request_dispatcher* dispatcher);
    void listen();
    void handle_connection(int id, sockpp::tcp_socket&& socket);

    std::string endpoint();
private:
    int server_id_;
    std::string host_;
    unsigned short port_;
    request_dispatcher* dispatcher_;
    ctpl::thread_pool thread_pool_;
};

} // namespace lrucache

#endif // LRUCACHE_NETWORK_MANAGER_H