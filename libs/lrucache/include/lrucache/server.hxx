#ifndef LRUCACHE_SERVER_H_
#define LRUCACHE_SERVER_H_

#include <memory>

#include "cache_config.hxx"

namespace lrucache {


class raft_manager;
class network_manager;
class request_dispatcher;
class request_handler;

class server {
public:
    server(cache_config config);

    void run();

    void set_commit_config_hook(); // TODO
    void set_pre_build_response_hook(); // TODO
    void set_post_build_response_hook(); // TODO
private:
    void register_handlers();

    cache_config config_;

    std::unique_ptr<raft_manager> raft_;

    std::unique_ptr<network_manager> network_manager_;

    std::unique_ptr<request_dispatcher> dispatcher_;

    std::unique_ptr<request_handler> handler_;
};

}

#endif // LRUCACHE_SERVER_H_