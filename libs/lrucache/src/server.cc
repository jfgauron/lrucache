#include "lrucache/server.hxx"

#include "raft/raft_server.hxx"
#include "cache/cache_server.hxx"

namespace lrucache {

server::server(cache_config config) : config_(config)
{
    int server_id = rand();
    raft_ = std::make_unique<raft_server>(config, server_id);
    cache_ = std::make_unique<cache_server>(config, server_id);
}

server::~server()
{
}

void server::init()
{

}

void server::run()
{
    cache_->run();
}

void server::backgroud_run()
{

}

}