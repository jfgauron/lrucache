#include "raft_server.hxx"

#include "in_memory_state_mgr.hxx"
#include "cache_state_machine.hxx"

#include <cstdlib>

namespace lrucache {

constexpr int ASIO_THREAD_POOL_SIZE = 8;
static bool ASYNC_SNAPSHOT_CREATION = false;

raft_server::raft_server(cache_config config, int server_id)
    : config_(config), server_id_(server_id)
{
    std::string endpoint = config.raft_endpoint();
    state_mgr_ = nuraft::cs_new<nuraft::inmem_state_mgr>(server_id_, endpoint);
    state_machine_ = nuraft::cs_new<cache_state_machine>(
            config, ASYNC_SNAPSHOT_CREATION);

    // ASIO options
    asio_service::options asio_opt;
    asio_opt.thread_pool_size_ = ASIO_THREAD_POOL_SIZE;

    // raft params
    raft_params params;
    params.heart_beat_interval_ = config.heart_beat_interval;
    params.election_timeout_lower_bound_ = config.election_timeout_lower_bound;
    params.election_timeout_upper_bound_ = config.election_timeout_upper_bound;
    params.reserved_log_items_ = config.reserved_log_items;
    params.snapshot_distance_ = config.snapshot_distance;
    params.client_req_timeout_ = config.client_req_timeout;
    params.return_method_ = raft_params::async_handler;
    
    // launch raft server
    m_instance_ = launcher_.init(state_machine_, state_mgr_, nullptr,
            config.raft_port, asio_opt, params);

    if (!m_instance_) {
        throw std::runtime_error("Failed to initialize raft server");
    }
}

raft_server::~raft_server() {}


};
