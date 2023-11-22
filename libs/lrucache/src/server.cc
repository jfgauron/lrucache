#include "lrucache/server.hxx"

#include "raft/raft_manager.hxx"
#include "network_manager.hxx"
#include "request_handler.hxx"

namespace lrucache {

server::server(cache_config config) : config_(config)
{
    int server_id = rand();
    raft_ = std::make_unique<raft_manager>(config, server_id);

    dispatcher_ = std::make_unique<request_dispatcher>(
        [this](const schema::Request* req, response_builder* builder) {
            return handler_->handle_unregistered_request(req, builder);
        }
    );

    network_manager_ = std::make_unique<network_manager>(
        server_id,
        config.host,
        config.port,
        dispatcher_.get());

    handler_ = std::make_unique<request_handler>(raft_.get());

    register_handlers();
}

void server::run()
{
    network_manager_->listen();
}

void server::register_handlers()
{
    dispatcher_->register_handler(
        schema::RequestContent::RequestContent_DiscoveryRequest,
        [this](const schema::Request* req, response_builder* builder) {
            return handler_->handle_discovery_request(req, builder);
        }
    );
}

} // namespace lrucache
